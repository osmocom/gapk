/* TI Calypso / Locosto (?) internal buffer format */

/* The buffer are actually 16 bits words and must be read as
 * such in the HW itself.
 * It's assumed here it was dumped by putting the 8 MSB first
 * and 8 LSB in the dumped files, for 33 bytes (thus dropping the
 * last 8 LSBs)
 */

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
#include <string.h>

#include <osmocom/codec/codec.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/utils.h>


static int
ti_hr_from_canon(uint8_t *dst, const uint8_t *src)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	memset(dst, 0x00, 33); /* Not even half the bits are written, so we pre-clear */

	voiced = (msb_get_bit(src, 34) << 1) | msb_get_bit(src, 35);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = bit_mapping[i];
		int di = i >= 95 ? i+4 : i;
		lsb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return 0;
}

static int
ti_hr_to_canon(uint8_t *dst, const uint8_t *src)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	voiced = (msb_get_bit(src, 94) << 1) | msb_get_bit(src, 93);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = i >= 95 ? i+4 : i;
		int di = bit_mapping[i];
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return 0;
}

const struct format_desc fmt_ti_hr = {
	.type			= FMT_TI_HR,
	.codec_type		= CODEC_HR,
	.name			= "ti-hr",
	.description		= "Texas Instrument HR TCH/H buffer format",

	.frame_len		= 33,
	.conv_from_canon	= ti_hr_from_canon,
	.conv_to_canon		= ti_hr_to_canon,
};


static int
ti_fr_from_canon(uint8_t *dst, const uint8_t *src)
{
	int i;

	dst[22] = dst[23] = 0x00; /* some bits won't be writter, pre-clear them */

	for (i=0; i<260; i++) {
		int si = gsm610_bitorder[i];
		int di = i >= 182 ? i+4 : i;
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return 0;
}

static int
ti_fr_to_canon(uint8_t *dst, const uint8_t *src)
{
	int i;

	dst[32] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<260; i++) {
		int si = i >= 182 ? i+4 : i;
		int di = gsm610_bitorder[i];
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return 0;
}

const struct format_desc fmt_ti_fr = {
	.type			= FMT_TI_FR,
	.codec_type		= CODEC_FR,
	.name			= "ti-fr",
	.description		= "Texas Instrument FR TCH/F buffer format",

	.frame_len		= 33,
	.conv_from_canon	= ti_fr_from_canon,
	.conv_to_canon		= ti_fr_to_canon,
};
