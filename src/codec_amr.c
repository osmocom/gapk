/* AMR (GSM 06.90) codec */
/* (C) 2017 Harald Welte <laforge@gnumonks.org> */

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

#include <gapk/codecs.h>
#include <gapk/benchmark.h>

#include "config.h"


#ifdef HAVE_OPENCORE_AMRNB

#include <stdlib.h>
#include <stdio.h>

#include <opencore-amrnb/interf_dec.h>
#include <opencore-amrnb/interf_enc.h>

struct codec_amr_state {
	void *encoder;
	void *decoder;
};


static void *
codec_amr_init(void)
{
	struct codec_amr_state *st;

	st = calloc(1, sizeof(*st));
	if (!st)
		return NULL;

	st->encoder = Encoder_Interface_init(0);
	st->decoder = Decoder_Interface_init();

	return (void *)st;
}

static void
codec_amr_exit(void *state)
{
	struct codec_amr_state *st = state;

	Decoder_Interface_exit(st->decoder);
	Encoder_Interface_exit(st->encoder);

	return;
}

static int
codec_amr_encode(void *state, uint8_t *cod, const uint8_t *pcm, unsigned int pcm_len)
{
	struct codec_amr_state *st = state;
	int rv;

	BENCHMARK_START;
	rv = Encoder_Interface_Encode(
		st->encoder,
		MR122,
		(const short*) pcm,
		(unsigned char*) cod,
		1
	);
	BENCHMARK_STOP(CODEC_EFR, 1);

	return rv;
}

static int
codec_amr_decode(void *state, uint8_t *pcm, const uint8_t *cod, unsigned int cod_len)
{
	struct codec_amr_state *st = state;

	printf("%s(): %u bytes in\n", __func__, cod_len);

	BENCHMARK_START;
	Decoder_Interface_Decode(
		st->decoder,
		(const unsigned char*) cod,
		(short *) pcm,
		0
	);
	BENCHMARK_STOP(CODEC_EFR, 0);

	return PCM_CANON_LEN;
}

#endif /* HAVE_OPENCORE_AMRNB */


const struct codec_desc codec_amr_desc = {
	.type = CODEC_AMR,
	.name = "amr",
	.description = "GSM 26.071 Adaptive Multi Rate codec",
	.canon_frame_len = 0,
#ifdef HAVE_OPENCORE_AMRNB
	.codec_enc_format_type = FMT_AMR_OPENCORE,
	.codec_dec_format_type = FMT_AMR_OPENCORE,
	.codec_init = codec_amr_init,
	.codec_exit = codec_amr_exit,
	.codec_encode = codec_amr_encode,
	.codec_decode = codec_amr_decode,
#endif
};
