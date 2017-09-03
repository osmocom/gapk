/* FR (GSM 06.10) codec */

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

#include <assert.h>

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/benchmark.h>
#include <osmocom/gapk/bench.h>

#include "config.h"


#ifdef HAVE_LIBGSM

/* Find header */
#ifdef HAVE_GSM_H
# include <gsm.h>
#elif HAVE_GSM_GSM_H
# include <gsm/gsm.h>
#else
# error "Can't find gsm.h header anywhere ..."
#endif

#include <string.h>


static void *
codec_fr_init(void)
{
	return (void *)gsm_create();
}

static void
codec_fr_exit(void *state)
{
	gsm_destroy( (gsm)state );
}

static int
codec_fr_encode(void *state, uint8_t *cod, const uint8_t *pcm, unsigned int pcm_len)
{
	gsm gh = (gsm)state;
	uint8_t pcm_b[PCM_CANON_LEN];	/* local copy as libgsm src isn't const ! */
	assert(pcm_len == PCM_CANON_LEN);
	memcpy(pcm_b, pcm, PCM_CANON_LEN);
	BENCHMARK_START;
	gsm_encode(gh, (gsm_signal*)pcm, (gsm_byte*)cod);
	BENCHMARK_STOP(CODEC_FR, 1);
	return FR_CANON_LEN;
}

static int
codec_fr_decode(void *state, uint8_t *pcm, const uint8_t *cod, unsigned int cod_len)
{
	gsm gh = (gsm)state;
	uint8_t cod_b[FR_CANON_LEN];	/* local copy as libgsm src isn't const ! */
	int rc;
	assert(cod_len == FR_CANON_LEN);
	memcpy(cod_b, cod, FR_CANON_LEN);
	BENCHMARK_START;
	rc = gsm_decode(gh, (gsm_byte*)cod_b, (gsm_signal*)pcm);
	BENCHMARK_STOP(CODEC_FR, 1);
	if (rc < 0)
		return rc;
	return PCM_CANON_LEN;
}

#endif /* HAVE_LIBGSM */


const struct osmo_gapk_codec_desc codec_fr_desc = {
	.type = CODEC_FR,
	.name = "fr",
	.description = "GSM 06.10 Full Rate codec (classic gsm codec)",
	.canon_frame_len = FR_CANON_LEN,
#ifdef HAVE_LIBGSM
	.codec_enc_format_type = FMT_GSM,
	.codec_dec_format_type = FMT_GSM,
	.codec_init = codec_fr_init,
	.codec_exit = codec_fr_exit,
	.codec_encode = codec_fr_encode,
	.codec_decode = codec_fr_decode,
#endif
};
