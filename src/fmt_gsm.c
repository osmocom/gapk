/* Classic .gsm files for FR */

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
#include <gapk/formats.h>

#define GSM_LEN	  33
#define GSM_MAGIC 0xd

/* convert canonical -> .gsm */
static int
gsm_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == FR_CANON_LEN);

	dst[0] = (GSM_MAGIC << 4) | (src[0] >> 4);
	for (i=1; i<GSM_LEN; i++)
		dst[i] = (src[i-1] << 4) | (src[i] >> 4);

	return GSM_LEN;
}

/* convert .gsm -> canonical */
static int
gsm_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == GSM_LEN);

	for (i=0; i<(GSM_LEN-1); i++)
		dst[i] = (src[i] << 4) | (src[i+1] >> 4);
	dst[GSM_LEN-1] = src[GSM_LEN-1] << 4;

	return FR_CANON_LEN;
}

const struct format_desc fmt_gsm = {
	.type			= FMT_GSM,
	.codec_type		= CODEC_FR,
	.name			= "gsm",
	.description		= "Classic .gsm file format (and RTP payload for FR according to RFC3551)",

	.frame_len		= GSM_LEN,
	.conv_from_canon	= gsm_from_canon,
	.conv_to_canon		= gsm_to_canon,
};
