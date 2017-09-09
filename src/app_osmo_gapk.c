/* Main */

/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * gapk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gapk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gapk.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <talloc.h>

#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <osmocom/core/application.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/socket.h>
#include <osmocom/core/utils.h>

#include <osmocom/gapk/common.h>
#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/benchmark.h>

/* The root talloc context of application */
TALLOC_CTX *app_root_ctx;

struct gapk_options
{
	const char *fname_in;
	struct {
		const char *hostname;
		uint16_t port;
	} rtp_in;
	const char *alsa_in;
	const struct osmo_gapk_format_desc *fmt_in;

	const char *fname_out;
	struct {
		const char *hostname;
		uint16_t port;
	} rtp_out;
	const char *alsa_out;
	const struct osmo_gapk_format_desc *fmt_out;

	int benchmark;
	int verbose;
};

struct gapk_state
{
	struct gapk_options opts;
	struct osmo_gapk_pq *pq;
	int exit;

	struct {
		struct {
			FILE *fh;
		} file;
		struct {
			int fd;
		} rtp;
	} in;

	struct {
		struct {
			FILE *fh;
		} file;
		struct {
			int fd;
		} rtp;
	} out;
};

/* Logging related routines */
enum {
	DAPP,
};

static struct log_info_cat gapk_log_info_cat[] = {
	[DAPP] = {
		.name = "DAPP",
		.description = "Application",
		.color = "\033[0;36m",
		.enabled = 1, .loglevel = LOGL_NOTICE,
	},
};

static const struct log_info gapk_log_info = {
	.cat = gapk_log_info_cat,
	.num_cat = ARRAY_SIZE(gapk_log_info_cat),
};


static void
print_help(char *progname)
{
	const struct osmo_gapk_codec_desc *codec;
	int i;

	/* Header */
	fprintf(stdout, "Usage: %s [options]\n", progname);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "  -i, --input-file=FILE\t\tInput file\n");
	fprintf(stdout, "  -I, --input-rtp=HOST/PORT\tInput RTP stream\n");
	fprintf(stdout, "  -o, --output-file=FILE\tOutput file\n");
	fprintf(stdout, "  -O, --output-rtp=HOST/PORT\tOutput RTP stream\n");
#ifdef HAVE_ALSA
	fprintf(stdout, "  -a, --input-alsa=dev\t\tInput ALSA stream\n");
	fprintf(stdout, "  -A, --output-alsa=dev\t\tOutput ALSA stream\n");
#endif
	fprintf(stdout, "  -f, --input-format=FMT\tInput format (see below)\n");
	fprintf(stdout, "  -g, --output-format=FMT\tOutput format (see below)\n");
	fprintf(stdout, "  -b, --enable-benchmark\tEnable codec benchmarking\n");
	fprintf(stdout, "  -v, --verbose\t\t\tEnable debug messages\n");
	fprintf(stdout, "\n");

	/* Print all codecs */
	fprintf(stdout, "Supported codecs:\n");
	fprintf(stdout, " name\tfmt enc dec\tdescription\n");

	for (i=CODEC_INVALID+1; i<_CODEC_MAX; i++) {
		codec = osmo_gapk_codec_get_from_type(i);
		fprintf(stdout, " %4s  %c   %c   %c  \t%s\n",
			codec->name,
			'*',
			codec->codec_encode ? '*' : ' ',
			codec->codec_decode ? '*' : ' ',
			codec->description
		);
	}

	fprintf(stdout, "\n");

	/* Print all formats */
	fprintf(stdout, "Supported formats:\n");
	
	for (i=FMT_INVALID+1; i<_FMT_MAX; i++) {
		const struct osmo_gapk_format_desc *fmt = osmo_gapk_fmt_get_from_type(i);
		fprintf(stdout, " %-19s %s\n",
			fmt->name,
			fmt->description
		);
	}

	fprintf(stdout, "\n");
}

static int
parse_host_port(const char *host_port, const char **host)
{
	char *dup = strdup(host_port);
	char *tok;

	if (!dup)
		return -ENOMEM;

	tok = strtok(dup, "/");
	if (!tok)
		return -EINVAL;
	*host = tok;

	tok = strtok(NULL, "/");
	if (!tok)
		return -EINVAL;

	return atoi(tok);
}

static int
parse_options(struct gapk_state *state, int argc, char *argv[])
{
	const struct option long_options[] = {
		{"input-file", 1, 0, 'i'},
		{"output-file", 1, 0, 'o'},
		{"input-rtp", 1, 0, 'I'},
		{"output-rtp", 1, 0, 'O'},
#ifdef HAVE_ALSA
		{"input-alsa", 1, 0, 'a'},
		{"output-alsa", 1, 0, 'A'},
#endif
		{"input-format", 1, 0, 'f'},
		{"output-format", 1, 0, 'g'},
		{"enable-benchmark", 0, 0, 'b'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
	};
	const char *short_options = "i:o:I:O:f:g:bvh"
#ifdef HAVE_ALSA
		"a:A:"
#endif
		;

	struct gapk_options *opt = &state->opts;

	/* Set some defaults */
	memset(opt, 0x00, sizeof(*opt));

	/* Parse */
	while (1) {
		int c, rv;
		int opt_idx;

		c = getopt_long(
			argc, argv, short_options, long_options, &opt_idx);
		if (c == -1)
			break;

		switch (c) {
		case 'i':
			opt->fname_in = optarg;
			break;

		case 'o':
			opt->fname_out = optarg;
			break;

		case 'I':
			rv = parse_host_port(optarg, &opt->rtp_in.hostname);
			if (rv < 0 || rv > 0xffff) {
				LOGP(DAPP, LOGL_ERROR, "Invalid port: %d\n", rv);
				return -EINVAL;
			}
			opt->rtp_in.port = rv;
			break;
#ifdef HAVE_ALSA
		case 'a':
			opt->alsa_in = optarg;
			break;

		case 'A':
			opt->alsa_out = optarg;
			break;
#endif
		case 'O':
			rv = parse_host_port(optarg, &opt->rtp_out.hostname);
			if (rv < 0 || rv > 0xffff) {
				LOGP(DAPP, LOGL_ERROR, "Invalid port: %d\n", rv);
				return -EINVAL;
			}
			opt->rtp_out.port = rv;
			break;

		case 'f':
			opt->fmt_in = osmo_gapk_fmt_get_from_name(optarg);
			if (!opt->fmt_in) {
				LOGP(DAPP, LOGL_ERROR, "Unsupported format: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'g':
			opt->fmt_out = osmo_gapk_fmt_get_from_name(optarg);
			if (!opt->fmt_out) {
				LOGP(DAPP, LOGL_ERROR, "Unsupported format: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'b':
			opt->benchmark = 1;
			break;

		case 'v':
			log_parse_category_mask(osmo_stderr_target, "DAPP");
			opt->verbose = 1;
			break;

		case 'h':
			return 1;

		default:
			LOGP(DAPP, LOGL_ERROR, "Unknown option\n");
			return -EINVAL;

		}
	}

	return 0;
}

static int
check_options(struct gapk_state *gs)
{
	/* Required formats */
	if (!gs->opts.fmt_in || !gs->opts.fmt_out) {
		LOGP(DAPP, LOGL_ERROR, "Input and output formats are required arguments\n");
		return -EINVAL;
	}

	/* Transcoding */
	if (gs->opts.fmt_in->codec_type != gs->opts.fmt_out->codec_type) {
		const struct osmo_gapk_codec_desc *codec;

		/* Check source codec */
		codec = osmo_gapk_codec_get_from_type(gs->opts.fmt_in->codec_type);
		if (!codec) {
			LOGP(DAPP, LOGL_ERROR, "Internal error: bad codec reference\n");
			return -EINVAL;
		}
		if ((codec->type != CODEC_PCM) && !codec->codec_decode) {
			LOGP(DAPP, LOGL_ERROR, "Decoding from '%s' codec is unsupported\n", codec->name);
			return -ENOTSUP;
		}

		/* Check destination codec */
		codec = osmo_gapk_codec_get_from_type(gs->opts.fmt_out->codec_type);
		if (!codec) {
			LOGP(DAPP, LOGL_ERROR, "Internal error: bad codec reference\n");
			return -EINVAL;
		}
		if ((codec->type != CODEC_PCM) && !codec->codec_encode) {
			LOGP(DAPP, LOGL_ERROR, "Encoding to '%s' codec is unsupported\n", codec->name);
			return -ENOTSUP;
		}
	}

	/* Check I/O combinations */
	int src_count = 0;
	int sink_count = 0;

	if (gs->opts.fname_in)
		src_count++;
	if (gs->opts.rtp_in.port)
		src_count++;
	if (gs->opts.alsa_in)
		src_count++;

	if (gs->opts.fname_out)
		sink_count++;
	if (gs->opts.rtp_out.port)
		sink_count++;
	if (gs->opts.alsa_out)
		sink_count++;

	if (src_count != 1 || sink_count != 1) {
		LOGP(DAPP, LOGL_ERROR, "You have to decide on "
			"a single input and a single output\n");
		return -EINVAL;
	}

	return 0;
}

static void
benchmark_dump(void)
{
	int i;

	for (i = 0; i < _CODEC_MAX; i++) {
		struct osmo_gapk_bench_cycles *bc;
		unsigned long long cycles;
		unsigned int frames;

		/* Check if there are benchmark data */
		bc = osmo_gapk_bench_codec[i];
		if (!bc)
			continue;

		if (bc->enc_used) {
			cycles = osmo_gapk_bench_get_cycles(i, 1);
			frames = osmo_gapk_bench_get_frames(i, 1);

			LOGP(DAPP, LOGL_NOTICE, "Codec %u (ENC): %llu cycles for %u frames"
				" => %llu cycles/frame\n", i, cycles,
				frames, cycles / frames);
		}

		if (bc->dec_used) {
			cycles = osmo_gapk_bench_get_cycles(i, 0);
			frames = osmo_gapk_bench_get_frames(i, 0);

			LOGP(DAPP, LOGL_NOTICE, "Codec %u (DEC): %llu cycles for %u frames"
				" => %llu cycles/frame\n", i, cycles,
				frames, cycles / frames);
		}
	}
}

static int
files_open(struct gapk_state *gs)
{
	LOGP(DAPP, LOGL_NOTICE, "Opening I/O streams\n");

	if (gs->opts.fname_in) {
		LOGP(DAPP, LOGL_NOTICE, "Using a file as source\n");
		gs->in.file.fh = fopen(gs->opts.fname_in, "rb");
		if (!gs->in.file.fh) {
			LOGP(DAPP, LOGL_ERROR, "Error while opening input file for reading\n");
			perror("fopen");
			return -errno;
		}
	} else if (gs->opts.rtp_in.port) {
		LOGP(DAPP, LOGL_NOTICE, "Using RTP as source\n");
		gs->in.rtp.fd = osmo_sock_init(AF_UNSPEC, SOCK_DGRAM,
						IPPROTO_UDP,
						gs->opts.rtp_in.hostname,
						gs->opts.rtp_in.port,
						OSMO_SOCK_F_BIND);
		if (gs->in.rtp.fd < 0) {
			LOGP(DAPP, LOGL_ERROR, "Error while opening input socket\n");
			return gs->in.rtp.fd;
		}
	} else if (gs->opts.alsa_in) {
		/* Do nothing, ALSA source does the initialization itself */
		LOGP(DAPP, LOGL_NOTICE, "Using ALSA as source\n");
	} else {
		LOGP(DAPP, LOGL_NOTICE, "Using stdin as source\n");
		gs->in.file.fh = stdin;
	}

	if (gs->opts.fname_out) {
		LOGP(DAPP, LOGL_NOTICE, "Using a file as sink\n");
		gs->out.file.fh = fopen(gs->opts.fname_out, "wb");
		if (!gs->out.file.fh) {
			LOGP(DAPP, LOGL_ERROR, "Error while opening output file for writing\n");
			perror("fopen");
			return -errno;
		}
	} else if (gs->opts.rtp_out.port) {
		LOGP(DAPP, LOGL_NOTICE, "Using RTP as sink\n");
		gs->out.rtp.fd = osmo_sock_init(AF_UNSPEC, SOCK_DGRAM,
						IPPROTO_UDP,
						gs->opts.rtp_out.hostname,
						gs->opts.rtp_out.port,
						OSMO_SOCK_F_CONNECT);
		if (gs->out.rtp.fd < 0) {
			LOGP(DAPP, LOGL_ERROR, "Error while opening output socket\n");
			return gs->out.rtp.fd;
		}
	} else if (gs->opts.alsa_out) {
		/* Do nothing, ALSA sink does the initialization itself */
		LOGP(DAPP, LOGL_NOTICE, "Using ALSA as sink\n");
	} else {
		LOGP(DAPP, LOGL_NOTICE, "Using stdout as sink\n");
		gs->out.file.fh = stdout;
	}

	return 0;
}

static void
files_close(struct gapk_state *gs)
{
	LOGP(DAPP, LOGL_NOTICE, "Closing I/O streams\n");

	if (gs->in.file.fh && gs->in.file.fh != stdin)
		fclose(gs->in.file.fh);
	if (gs->in.rtp.fd >= 0)
		close(gs->in.rtp.fd);
	if (gs->out.file.fh && gs->out.file.fh != stdout)
		fclose(gs->out.file.fh);
	if (gs->out.rtp.fd >= 0)
		close(gs->out.rtp.fd);
}

static int
handle_headers(struct gapk_state *gs)
{
	int rv;
	unsigned int len;

	/* Input file header (remove & check it) */
	len = gs->opts.fmt_in->header_len;
	if (len && gs->in.file.fh) {
		uint8_t *buf;
		
		buf = talloc_size(app_root_ctx, len);
		if (!buf)
			return -ENOMEM;

		rv = fread(buf, len, 1, gs->in.file.fh);
		if ((rv != 1) ||
		    memcmp(buf, gs->opts.fmt_in->header, len))
		{
			LOGP(DAPP, LOGL_ERROR, "Invalid header in input file");
			talloc_free(buf);
			return -EINVAL;
		}

		talloc_free(buf);
	}

	/* Output file header (write it) */
	len = gs->opts.fmt_out->header_len;
	if (len && gs->out.file.fh) {
		rv = fwrite(gs->opts.fmt_out->header, len, 1, gs->out.file.fh);
		if (rv != 1)
			return -ENOSPC;
	}

	return 0;
}

static int
make_processing_chain(struct gapk_state *gs)
{
	const struct osmo_gapk_format_desc *fmt_in, *fmt_out;
	const struct osmo_gapk_codec_desc *codec_in, *codec_out;

	int need_dec, need_enc;

	LOGP(DAPP, LOGL_NOTICE, "Creating a processing queue\n");

	fmt_in  = gs->opts.fmt_in;
	fmt_out = gs->opts.fmt_out;

	codec_in  = osmo_gapk_codec_get_from_type(fmt_in->codec_type);
	codec_out = osmo_gapk_codec_get_from_type(fmt_out->codec_type);

	need_dec = (fmt_in->codec_type != CODEC_PCM) &&
	           (fmt_in->codec_type != fmt_out->codec_type);
	need_enc = (fmt_out->codec_type != CODEC_PCM) &&
	           (fmt_out->codec_type != fmt_in->codec_type);

	/* File read */
	if (gs->in.file.fh)
		osmo_gapk_pq_queue_file_input(gs->pq, gs->in.file.fh, fmt_in->frame_len);
	else if (gs->in.rtp.fd != -1)
		osmo_gapk_pq_queue_rtp_input(gs->pq, gs->in.rtp.fd, fmt_in->frame_len);
#ifdef HAVE_ALSA
	else if (gs->opts.alsa_in)
		osmo_gapk_pq_queue_alsa_input(gs->pq, gs->opts.alsa_in, fmt_in->frame_len);
#endif
	else {
		LOGP(DAPP, LOGL_ERROR, "Unknown/invalid input\n");
		return -1;
	}

	/* Decoding to PCM ? */
	if (need_dec)
	{
		/* Convert input to decoder input fmt */
		if (fmt_in->type != codec_in->codec_dec_format_type)
		{
			const struct osmo_gapk_format_desc *fmt_dec;

			fmt_dec = osmo_gapk_fmt_get_from_type(codec_in->codec_dec_format_type);
			if (!fmt_dec) {
				LOGP(DAPP, LOGL_ERROR, "Cannot determine decoder input format "
					"for codec %s\n", codec_in->name);
				return -EINVAL;
			}

			osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_in, 0);
			osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_dec, 1);
		}

		/* Do decoding */
		osmo_gapk_pq_queue_codec(gs->pq, codec_in, 0);

		/* Allocate memory for benchmarking */
		if (gs->opts.benchmark)
			osmo_gapk_bench_enable(fmt_in->codec_type);
	}
	else if (fmt_in->type != fmt_out->type)
	{
		/* Convert input to canonical fmt */
		osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_in, 0);
	}

	/* Encoding from PCM ? */
	if (need_enc)
	{
		/* Do encoding */
		osmo_gapk_pq_queue_codec(gs->pq, codec_out, 1);

		/* Allocate memory for benchmarking */
		if (gs->opts.benchmark)
			osmo_gapk_bench_enable(fmt_out->codec_type);

		/* Convert encoder output to output fmt */
		if (fmt_out->type != codec_out->codec_enc_format_type)
		{
			const struct osmo_gapk_format_desc *fmt_enc;

			fmt_enc = osmo_gapk_fmt_get_from_type(codec_out->codec_enc_format_type);
			if (!fmt_enc) {
				LOGP(DAPP, LOGL_ERROR, "Cannot determine encoder output format "
					"for codec %s\n", codec_out->name);
				return -EINVAL;
			}

			osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_enc, 0);
			osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_out, 1);
		}
	}
	else if (fmt_in->type != fmt_out->type)
	{
		/* Convert canonical to output fmt */
		osmo_gapk_pq_queue_fmt_convert(gs->pq, fmt_out, 1);
	}

	/* File write */
	if (gs->out.file.fh)
		osmo_gapk_pq_queue_file_output(gs->pq, gs->out.file.fh, fmt_out->frame_len);
	else if (gs->out.rtp.fd != -1)
		osmo_gapk_pq_queue_rtp_output(gs->pq, gs->out.rtp.fd, fmt_out->frame_len);
#ifdef HAVE_ALSA
	else if (gs->opts.alsa_out)
		osmo_gapk_pq_queue_alsa_output(gs->pq, gs->opts.alsa_out, fmt_out->frame_len);
#endif
	else {
		LOGP(DAPP, LOGL_ERROR, "Unknown/invalid output\n");
		return -1;
	}

	return 0;
}

static int
run(struct gapk_state *gs)
{
	struct osmo_gapk_pq_item *item;
	int rv, frames;

	rv = osmo_gapk_pq_prepare(gs->pq);
	if (rv)
		return rv;

	for (frames = 0; !gs->exit; frames++) {
		rv = osmo_gapk_pq_execute(gs->pq);
		if (rv)
			break;
	}

	LOGP(DAPP, LOGL_NOTICE, "Processed %d frames\n", frames);

	/* Wait for sink to process buffers */
	item = llist_last_entry(&gs->pq->items, struct osmo_gapk_pq_item, list);
	if (item->wait && !gs->exit) {
		LOGP(DAPP, LOGL_NOTICE, "Waiting for sink to finish...\n");
		while (item->wait(item->state))
			continue;
	}

	return frames > 0 ? 0 : rv;
}


static struct gapk_state _gs, *gs = &_gs;

static void app_shutdown(void)
{
	/* Close source / destination files */
	files_close(gs);

	/* Release processing queue */
	osmo_gapk_pq_destroy(gs->pq);

	/* Print benchmarking results, if enabled */
	benchmark_dump();

	/* Free memory taken by benchmark data */
	osmo_gapk_bench_free();

	if (gs->opts.verbose)
		talloc_report_full(app_root_ctx, stderr);
}

static void signal_handler(int signal)
{
	fprintf(stderr, "signal %u received\n", signal);

	switch (signal) {
	case SIGINT:
		if (gs->exit++) {
			app_shutdown();
			exit(0);
		}
		break;
	case SIGABRT:
	case SIGUSR1:
	case SIGUSR2:
		talloc_report_full(app_root_ctx, stderr);
	default:
		break;
	}
}


int main(int argc, char *argv[])
{
	int rv;

	/* Init talloc memory management system */
	app_root_ctx = talloc_init("osmo-gapk root context");
	osmo_gapk_set_talloc_ctx(app_root_ctx);

	/* Init Osmocom logging framework */
	osmo_init_logging(&gapk_log_info);
	/* and GAPK logging wrapper */
	osmo_gapk_log_init(DAPP);

	/* Clear state */
	memset(gs, 0x00, sizeof(struct gapk_state));
	gs->in.rtp.fd = -1;
	gs->out.rtp.fd = -1;

	/* Parse / check options */
	rv = parse_options(gs, argc, argv);
	if (rv > 0) {
		print_help(argv[0]);
		return 0;
	}
	if (rv < 0)
		return rv;

	/* Check request */
	rv = check_options(gs);
	if (rv)
		return rv;

	/* Create processing queue */
	gs->pq = osmo_gapk_pq_create("main");
	if (!gs->pq) {
		rv = -ENOMEM;
		LOGP(DAPP, LOGL_ERROR, "Error creating processing queue\n");
		goto error;
	}

	/* Open source / destination files */
	rv = files_open(gs);
	if (rv) {
		LOGP(DAPP, LOGL_ERROR, "Error opening file(s)\n");
		goto error;
	}

	/* Handle input/output headers */
	rv = handle_headers(gs);
	if (rv) {
		LOGP(DAPP, LOGL_ERROR, "Error handling header(s)\n");
		goto error;
	}

	/* Make processing chain */
	rv = make_processing_chain(gs);
	if (rv) {
		LOGP(DAPP, LOGL_ERROR, "Error making processing chain\n");
		goto error;
	}

	signal(SIGINT, &signal_handler);
	signal(SIGABRT, &signal_handler);
	signal(SIGUSR1, &signal_handler);
	signal(SIGUSR2, &signal_handler);

	/* Run the processing queue */
	LOGP(DAPP, LOGL_NOTICE, "Init complete, starting processing queue...\n");
	rv = run(gs);

error:
	app_shutdown();
	return rv;
}
