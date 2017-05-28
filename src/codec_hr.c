/* HR (GSM 06.20) codec */

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

#include <gapk/codecs.h>
#include <gapk/benchmark.h>

#include "config.h"


#ifdef HAVE_LIBGSMHR

#include <gsmhr/gsmhr.h>

static void *
codec_hr_init(void)
{
	return (void *)gsmhr_init();
}

static void
codec_hr_exit(void *_state)
{
	struct gsmhr *state = _state;
	gsmhr_exit(state);
}

static int
codec_hr_encode(void *_state, uint8_t *cod, const uint8_t *pcm, unsigned int pcm_len)
{
	struct gsmhr *state = _state;
	int rc;
	assert(pcm_len == PCM_CANON_LEN);
	BENCHMARK_START;
	rc = gsmhr_encode(state, (int16_t *)cod, (const int16_t *)pcm);
	BENCHMARK_STOP(CODEC_HR, 1);
	if (rc < 0)
		return rc;
	return HR_REF_ENC_LEN;
}

static int
codec_hr_decode(void *_state, uint8_t *pcm, const uint8_t *cod, unsigned int cod_len)
{
	struct gsmhr *state = _state;
	int rc;
	assert(cod_len == HR_REF_DEC_LEN);
	BENCHMARK_START;
	rc = gsmhr_decode(state, (int16_t *)pcm, (const int16_t *)cod);
	BENCHMARK_STOP(CODEC_HR, 0);
	if (rc < 0)
		return rc;
	return PCM_CANON_LEN;
}

#endif /* HAVE_LIBGSMHR */


const struct codec_desc codec_hr_desc = {
	.type = CODEC_HR,
	.name = "hr",
	.description = "GSM 06.20 Half Rate codec",
	.canon_frame_len = HR_CANON_LEN,
#ifdef HAVE_LIBGSMHR
	.codec_enc_format_type = FMT_HR_REF_ENC,
	.codec_dec_format_type = FMT_HR_REF_DEC,
	.codec_init = codec_hr_init,
	.codec_exit = codec_hr_exit,
	.codec_encode = codec_hr_encode,
	.codec_decode = codec_hr_decode,
#endif
};
