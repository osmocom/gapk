/* RAW PCM output */

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


static int
rawpcm_s16le_from_canon(uint8_t *dst, const uint8_t *src)
{
	int i;
	const uint16_t *samples = (const uint16_t *)src;

	for (i=0; i<160; i++) {
		uint16_t w = samples[i];
		dst[(i<<1)  ] =  w       & 0xff;
		dst[(i<<1)+1] = (w >> 8) & 0xff;
	}

	return 0;
}

static int
rawpcm_s16le_to_canon(uint8_t *dst, const uint8_t *src)
{
	int i;
	uint16_t *samples = (uint16_t *)dst;

	for (i=0; i<160; i++)
		samples[i] = (src[(i<<1)+1] << 8) | src[(i<<1)];

	return 0;
}

const struct format_desc fmt_rawpcm_s16le = {
	.type			= FMT_RAWPCM_S16LE,
	.codec_type		= CODEC_PCM,
	.name			= "rawpcm-s16le",
	.description		= "Raw PCM samples Signed 16 bits little endian",

	.frame_len		= PCM_CANON_LEN,
	.conv_from_canon	= rawpcm_s16le_from_canon,
	.conv_to_canon		= rawpcm_s16le_to_canon,
};
