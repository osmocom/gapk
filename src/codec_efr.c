/* EFR (GSM 06.60) codec */

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

#include "config.h"


#ifdef HAVE_OPENCORE_AMRNB

#include <stdlib.h>

#include <interf_dec.h>
#include <interf_enc.h>


struct codec_efr_state {
	void *encoder;
	void *decoder;
};


static void *
codec_efr_init(void)
{
	struct codec_efr_state *st;

	st = calloc(1, sizeof(*st));
	if (!st)
		return NULL;

	st->encoder = Encoder_Interface_init(0);
	st->decoder = Decoder_Interface_init();

	return (void *)st;
}

static void
codec_efr_exit(void *state)
{
	struct codec_efr_state *st = state;

	Decoder_Interface_exit(st->decoder);
	Encoder_Interface_exit(st->encoder);

	return;
}

static int
codec_efr_encode(void *state, uint8_t *cod, const uint8_t *pcm)
{
	struct codec_efr_state *st = state;
	int rv;

	rv = Encoder_Interface_Encode(
		st->encoder,
		MR122,
		(const short*) pcm,
		(unsigned char*) cod,
		1
	);

	return rv != 32;
}

static int
codec_efr_decode(void *state, uint8_t *pcm, const uint8_t *cod)
{
	struct codec_efr_state *st = state;

	Decoder_Interface_Decode(
		st->decoder,
		(const unsigned char*) cod,
		(short *) pcm,
		0
	);

	return 0;
}

#endif /* HAVE_OPENCORE_AMRNB */


const struct codec_desc codec_efr_desc = {
	.type = CODEC_EFR,
	.name = "efr",
	.description = "GSM 06.60 Enhanced Full Rate codec",
	.canon_frame_len = 31,
#ifdef HAVE_OPENCORE_AMRNB
	.codec_enc_format_type = FMT_AMR_EFR,
	.codec_dec_format_type = FMT_AMR_EFR,
	.codec_init = codec_efr_init,
	.codec_exit = codec_efr_exit,
	.codec_encode = codec_efr_encode,
	.codec_decode = codec_efr_decode,
#endif
};
