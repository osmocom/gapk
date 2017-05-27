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

#include <gapk/codecs.h>
#include <gapk/formats.h>


#define GSM_MAGIC 0xd

/* convert canonical -> .gsm */
static int
gsm_from_canon(uint8_t *dst, const uint8_t *src)
{
	int i;

	dst[0] = (GSM_MAGIC << 4) | (src[0] >> 4);
	for (i=1; i<33; i++)
		dst[i] = (src[i-1] << 4) | (src[i] >> 4);

	return 0;
}

/* convert .gsm -> canonical */
static int
gsm_to_canon(uint8_t *dst, const uint8_t *src)
{
	int i;

	for (i=0; i<32; i++)
		dst[i] = (src[i] << 4) | (src[i+1] >> 4);
	dst[32] = src[32] << 4;

	return 0;
}

const struct format_desc fmt_gsm = {
	.type			= FMT_GSM,
	.codec_type		= CODEC_FR,
	.name			= "gsm",
	.description		= "Classic .gsm file format",

	.frame_len		= 33,
	.conv_from_canon	= gsm_from_canon,
	.conv_to_canon		= gsm_to_canon,
};
