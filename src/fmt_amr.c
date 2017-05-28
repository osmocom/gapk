/* Classic .amr file format. Be warned, actaully contains EFR ;) */

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

#include <stdint.h>
#include <assert.h>

#include <osmocom/codec/codec.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/utils.h>

#define EFR_LEN		31

/* conversion function: .amr file -> canonical format */
static int
amr_efr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == EFR_CANON_LEN);

	dst[ 0] = 0x3c;
	dst[31] = 0x00;	/* last nibble won't written, pre-clear it */

	for (i=0; i<244; i++) {
		int si = gsm690_12_2_bitorder[i];
		int di = i;
		msb_put_bit(&dst[1], di, msb_get_bit(src, si));
	}

	return EFR_LEN;
}

/* conversion function: canonical format -> .amr file */
static int
amr_efr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == EFR_LEN);

	if (src[0] != 0x3c)
		return -1;

	dst[30] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<244; i++) {
		int si = i;
		int di = gsm690_12_2_bitorder[i];
		msb_put_bit(dst, di, msb_get_bit(&src[1], si));
	}

	return EFR_CANON_LEN;
}

const struct format_desc fmt_amr_efr = {
	.type			= FMT_AMR_EFR,
	.codec_type		= CODEC_EFR,
	.name			= "amr-efr",
	.description		= "Classic .amr file containing EFR (=AMR 12.2k) data",

	.frame_len		= EFR_LEN,
	.conv_from_canon	= amr_efr_from_canon,
	.conv_to_canon		= amr_efr_to_canon,

	.header_len		= 6,
	.header			= (const uint8_t *) "#!AMR\x0a",
};
