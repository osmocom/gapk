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
#include <assert.h>

#include <osmocom/codec/codec.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/utils.h>

#define TI_LEN	33

static int
ti_hr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	assert(src_len == HR_CANON_LEN);

	memset(dst, 0x00, TI_LEN); /* Not even half the bits are written, so we pre-clear */

	voiced = (msb_get_bit(src, 34) << 1) | msb_get_bit(src, 35);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = bit_mapping[i];
		int di = i >= 95 ? i+4 : i;
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return TI_LEN;
}

static int
ti_hr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	assert(src_len == TI_LEN);

	voiced = (msb_get_bit(src, 94) << 1) | msb_get_bit(src, 93);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = i >= 95 ? i+4 : i;
		int di = bit_mapping[i];
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return HR_CANON_LEN;
}

const struct format_desc fmt_ti_hr = {
	.type			= FMT_TI_HR,
	.codec_type		= CODEC_HR,
	.name			= "ti-hr",
	.description		= "Texas Instrument HR TCH/H buffer format",

	.frame_len		= TI_LEN,
	.conv_from_canon	= ti_hr_from_canon,
	.conv_to_canon		= ti_hr_to_canon,
};


static int
ti_fr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == FR_CANON_LEN);

	dst[22] = dst[23] = 0x00; /* some bits won't be writter, pre-clear them */

	for (i=0; i<260; i++) {
		int si = gsm610_bitorder[i];
		int di = i >= 182 ? i+4 : i;
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return TI_LEN;
}

static int
ti_fr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == TI_LEN);

	dst[32] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<260; i++) {
		int si = i >= 182 ? i+4 : i;
		int di = gsm610_bitorder[i];
		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return FR_CANON_LEN;
}

const struct format_desc fmt_ti_fr = {
	.type			= FMT_TI_FR,
	.codec_type		= CODEC_FR,
	.name			= "ti-fr",
	.description		= "Texas Instrument FR TCH/F buffer format",

	.frame_len		= TI_LEN,
	.conv_from_canon	= ti_fr_from_canon,
	.conv_to_canon		= ti_fr_to_canon,
};


static int
ti_efr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == EFR_CANON_LEN);

	memset(dst, 0x00, 33); /* pre-clear */

	for (i=0; i<244; i++) {
		int si, tsi = gsm660_bitorder[i];
		int di = i >= 182 ? i+4 : i;

		if (tsi < 71)
			si = tsi;
		else if (tsi < 73)
			continue; /* repeated bits */
		else if (tsi < 123)
			si = tsi - 2;
		else if (tsi < 125)
			continue; /* repeated bits */
		else if (tsi < 178)
			si = tsi - 4;
		else if (tsi < 180)
			continue; /* repeated bits */
		else if (tsi < 230)
			si = tsi - 6;
		else if (tsi < 232)
			continue; /* repeated bits */
		else if (tsi < 252)
			si = tsi - 8;
		else
			continue; /* CRC poly */

		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return TI_LEN;
}

static int
ti_efr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == TI_LEN);

	dst[30] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<244; i++) {
		int si = i >= 182 ? i+4 : i;
		int di, tdi = gsm660_bitorder[i];

		if (tdi < 71)
			di = tdi;
		else if (tdi < 73)
			continue; /* repeated bits */
		else if (tdi < 123)
			di = tdi - 2;
		else if (tdi < 125)
			continue; /* repeated bits */
		else if (tdi < 178)
			di = tdi - 4;
		else if (tdi < 180)
			continue; /* repeated bits */
		else if (tdi < 230)
			di = tdi - 6;
		else if (tdi < 232)
			continue; /* repeated bits */
		else if (tdi < 252)
			di = tdi - 8;
		else
			continue; /* CRC poly */

		msb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return EFR_CANON_LEN;
}

const struct format_desc fmt_ti_efr = {
	.type			= FMT_TI_EFR,
	.codec_type		= CODEC_EFR,
	.name			= "ti-efr",
	.description		= "Texas Instrument EFR TCH/F buffer format",

	.frame_len		= TI_LEN,
	.conv_from_canon	= ti_efr_from_canon,
	.conv_to_canon		= ti_efr_to_canon,
};
