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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/procqueue.h>


struct gapk_options
{
	const char *fname_in;
	const struct format_desc *fmt_in;

	const char *fname_out;
	const struct format_desc *fmt_out;
};

struct gapk_state
{
	struct gapk_options opts;

	struct pq *pq;

	FILE *fh_in;
	FILE *fh_out;
};


static void
print_help(char *progname)
{
	int i;

	/* Header */
	fprintf(stdout, "Usage: %s [options]\n", progname);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "  -i, --input-file=FILE\t\tInput file\n");
	fprintf(stdout, "  -o, --output-file=FILE\tOutput file\n");
	fprintf(stdout, "  -f, --input-format=FMT\tInput format (see below)\n");
	fprintf(stdout, "  -g, --output-format=FMT\tOutput format (see below)\n");
	fprintf(stdout, "\n");

	/* Print all codecs */
	fprintf(stdout, "Supported codecs:\n");
	fprintf(stdout, " name\tfmt enc dec\tdescription\n");

	for (i=CODEC_INVALID+1; i<_CODEC_MAX; i++) {
		const struct codec_desc *codec = codec_get_from_type(i);
		fprintf(stdout, " %s\t %c   %c   %c\t%s\n",
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
		const struct format_desc *fmt = fmt_get_from_type(i);
		fprintf(stdout, " %s%s%s\t%s\n",
			fmt->name,
			strlen(fmt->name) <  7 ? "\t" : "",
			strlen(fmt->name) < 15 ? "\t" : "",
			fmt->description
		);
	}

	fprintf(stdout, "\n");
}

static int
parse_options(struct gapk_state *state, int argc, char *argv[])
{
	const struct option long_options[] = {
		{"input-file", 1, 0, 'i'},
		{"output-file", 1, 0, 'o'},
		{"input-format", 1, 0, 'f'},
		{"output-format", 1, 0, 'g'},
		{"help", 0, 0, 'h'},
	};
	const char *short_options = "i:o:f:g:h";

	struct gapk_options *opt = &state->opts;

	/* Set some defaults */
	memset(opt, 0x00, sizeof(*opt));

	/* Parse */
	while (1) {
		int c;
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

		case 'f':
			opt->fmt_in = fmt_get_from_name(optarg);
			if (!opt->fmt_in) {
				fprintf(stderr, "[!] Unsupported format: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'g':
			opt->fmt_out = fmt_get_from_name(optarg);
			if (!opt->fmt_out) {
				fprintf(stderr, "[!] Unsupported format: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'h':
			return 1;

		default:
			fprintf(stderr, "[+] Unknown option\n");
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
		fprintf(stderr, "[!] Input and output formats are required arguments !\n");
		return -EINVAL;
	}

	/* Transcoding */
	if (gs->opts.fmt_in->codec_type != gs->opts.fmt_out->codec_type) {
		const struct codec_desc *codec;

		/* Check source codec */
		codec = codec_get_from_type(gs->opts.fmt_in->codec_type);
		if (!codec) {
			fprintf(stderr, "[!] Internal error: bad codec reference\n");
			return -EINVAL;
		}
		if ((codec->type != CODEC_PCM) && !codec->codec_decode) {
			fprintf(stderr, "[!] Decoding from '%s' codec is unsupported\n", codec->name);
			return -ENOTSUP;
		}

		/* Check destination codec */
		codec = codec_get_from_type(gs->opts.fmt_out->codec_type);
		if (!codec) {
			fprintf(stderr, "[!] Internal error: bad codec reference\n");
			return -EINVAL;
		}
		if ((codec->type != CODEC_PCM) && !codec->codec_encode) {
			fprintf(stderr, "[!] Encoding to '%s' codec is unsupported\n", codec->name);
			return -ENOTSUP;
		}
	}

	return 0;
}

static int
files_open(struct gapk_state *gs)
{
	if (gs->opts.fname_in) {
		gs->fh_in = fopen(gs->opts.fname_in, "rb");
		if (!gs->fh_in) {
			fprintf(stderr, "[!] Error while opening input file for reading\n");
			perror("fopen");
			return -errno;
		}
	} else
		gs->fh_in = stdin;

	if (gs->opts.fname_out) {
		gs->fh_out = fopen(gs->opts.fname_out, "wb");
		if (!gs->fh_out) {
			fprintf(stderr, "[!] Error while opening output file for writing\n");
			perror("fopen");
			return -errno;
		}
	} else
		gs->fh_out = stdout;

	return 0;
}

static void
files_close(struct gapk_state *gs)
{
	if (gs->fh_in && gs->fh_in != stdin)
		fclose(gs->fh_in);
	if (gs->fh_out && gs->fh_out != stdout)
		fclose(gs->fh_out);
}

static int
handle_headers(struct gapk_state *gs)
{
	int rv;
	unsigned int len;

	/* Input file header (remove & check it) */
	len = gs->opts.fmt_in->header_len;
	if (len) {
		uint8_t *buf;
		
		buf = malloc(len);
		if (!buf)
			return -ENOMEM;

		rv = fread(buf, len, 1, gs->fh_in);
		if ((rv != 1) ||
		    memcmp(buf, gs->opts.fmt_in->header, len))
		{
			free(buf);
			fprintf(stderr, "[!] Invalid header in input file");
			return -EINVAL;
		}

		free(buf);
	}

	/* Output file header (write it) */
	len = gs->opts.fmt_out->header_len;
	if (len) {
		rv = fwrite(gs->opts.fmt_out->header, len, 1, gs->fh_out);
		if (rv != 1)
			return -ENOSPC;
	}

	return 0;
}

static int
make_processing_chain(struct gapk_state *gs)
{
	const struct format_desc *fmt_in, *fmt_out;
	const struct codec_desc *codec_in, *codec_out;

	int need_dec, need_enc;

	fmt_in  = gs->opts.fmt_in;
	fmt_out = gs->opts.fmt_out;

	codec_in  = codec_get_from_type(fmt_in->codec_type);
	codec_out = codec_get_from_type(fmt_out->codec_type);

	need_dec = (fmt_in->codec_type != CODEC_PCM) &&
	           (fmt_in->codec_type != fmt_out->codec_type);
	need_enc = (fmt_out->codec_type != CODEC_PCM) &&
	           (fmt_out->codec_type != fmt_in->codec_type);

	/* File read */
	pq_queue_file_input(gs->pq, gs->fh_in, fmt_in->frame_len);

	/* Decoding to PCM ? */
	if (need_dec)
	{
		/* Convert input to decoder input fmt */
		if (fmt_in->type != codec_in->codec_dec_format_type)
		{
			const struct format_desc *fmt_dec;

			fmt_dec = fmt_get_from_type(codec_in->codec_dec_format_type);
			if (!fmt_dec)
				return -EINVAL;

			pq_queue_fmt_convert(gs->pq, fmt_in, 0);
			pq_queue_fmt_convert(gs->pq, fmt_dec, 1);
		}

		/* Do decoding */
		pq_queue_codec(gs->pq, codec_in, 0);
	}
	else if (fmt_in->type != fmt_out->type)
	{
		/* Convert input to canonical fmt */
		pq_queue_fmt_convert(gs->pq, fmt_in, 0);
	}

	/* Encoding from PCM ? */
	if (need_enc)
	{
		/* Do encoding */
		pq_queue_codec(gs->pq, codec_out, 1);

		/* Convert encoder output to output fmt */
		if (fmt_out->type != codec_out->codec_enc_format_type)
		{
			const struct format_desc *fmt_enc;

			fmt_enc = fmt_get_from_type(codec_out->codec_enc_format_type);
			if (!fmt_enc)
				return -EINVAL;

			pq_queue_fmt_convert(gs->pq, fmt_enc, 0);
			pq_queue_fmt_convert(gs->pq, fmt_out, 1);
		}
	}
	else if (fmt_in->type != fmt_out->type)
	{
		/* Convert canonical to output fmt */
		pq_queue_fmt_convert(gs->pq, fmt_out, 1);
	}

	/* File write */
	pq_queue_file_output(gs->pq, gs->fh_out, fmt_out->frame_len);

	return 0;
}

static int
run(struct gapk_state *gs)
{
	int rv, frames;

	rv = pq_prepare(gs->pq);
	if (rv)
		return rv;

	for (frames=0; !(rv = pq_execute(gs->pq)); frames++);

	fprintf(stderr, "[+] Processed %d frames\n", frames);

	return frames > 0 ? 0 : rv;
}

int main(int argc, char *argv[])
{
	struct gapk_state _gs, *gs = &_gs;
	int rv;

	/* Clear state */
	memset(gs, 0x00, sizeof(struct gapk_state));

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
	gs->pq = pq_create();
	if (!gs->pq) {
		rv = -ENOMEM;
		goto error;
	}

	/* Open source / destination files */
	rv = files_open(gs);
	if (rv)
		goto error;

	/* Handle input/output headers */
	rv = handle_headers(gs);
	if (rv)
		goto error;

	/* Make processing chain */
	rv = make_processing_chain(gs);
	if (rv)
		goto error;

	/* Run the processing queue */
	rv = run(gs);

error:
	/* Close source / destination files */
	files_close(gs);

	/* Release processing queue */
	pq_destroy(gs->pq);
	
	return rv;
}
