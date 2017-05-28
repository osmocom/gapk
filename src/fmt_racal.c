/* Racal 6103E TCH recordings format */

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

#define RACAL_HR_LEN	14
#define RACAL_FR_LEN	33
#define RACAL_EFR_LEN	31

static int
racal_hr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	assert(src_len == HR_CANON_LEN);

	voiced = (msb_get_bit(src, 34) << 1) | msb_get_bit(src, 35);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = bit_mapping[i];
		int di = i;
		lsb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return RACAL_HR_LEN;
}

static int
racal_hr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i, voiced;
	const uint16_t *bit_mapping;

	assert(src_len == HR_CANON_LEN);

	voiced = (lsb_get_bit(src, 94) << 1) | lsb_get_bit(src, 93);

	bit_mapping = voiced ?
		&gsm620_voiced_bitorder[0] :
		&gsm620_unvoiced_bitorder[0] ;

	for (i=0; i<112; i++) {
		int si = i;
		int di = bit_mapping[i];
		msb_put_bit(dst, di, lsb_get_bit(src, si));
	}

	return RACAL_HR_LEN;
}

const struct format_desc fmt_racal_hr = {
	.type			= FMT_RACAL_HR,
	.codec_type		= CODEC_HR,
	.name			= "racal-hr",
	.description		= "Racal HR TCH/H recording",

	.frame_len		= RACAL_HR_LEN,
	.conv_from_canon	= racal_hr_from_canon,
	.conv_to_canon		= racal_hr_to_canon,
};


static int
racal_fr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == FR_CANON_LEN);

	dst[32] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<260; i++) {
		int si = gsm610_bitorder[i];
		int di = i;
		lsb_put_bit(dst, di, msb_get_bit(src, si));
	}

	return RACAL_FR_LEN;
}

static int 
racal_fr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == RACAL_FR_LEN);

	dst[32] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<260; i++) {
		int si = i;
		int di = gsm610_bitorder[i];
		msb_put_bit(dst, di, lsb_get_bit(src, si));
	}

	return FR_CANON_LEN;
}

const struct format_desc fmt_racal_fr = {
	.type			= FMT_RACAL_FR,
	.codec_type		= CODEC_FR,
	.name			= "racal-fr",
	.description		= "Racal FR TCH/F recording",

	.frame_len		= RACAL_FR_LEN,
	.conv_from_canon	= racal_fr_from_canon,
	.conv_to_canon		= racal_fr_to_canon,
};


static int
racal_efr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == EFR_CANON_LEN);

	dst[30] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<244; i++)
		lsb_put_bit(dst, i, msb_get_bit(src, i));

	return RACAL_EFR_LEN;
}

static int
racal_efr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	assert(src_len == RACAL_EFR_LEN);

	dst[30] = 0x00; /* last nibble won't written, pre-clear it */

	for (i=0; i<244; i++)
		msb_put_bit(dst, i, lsb_get_bit(src, i));

	return EFR_CANON_LEN;
}

const struct format_desc fmt_racal_efr = {
	.type			= FMT_RACAL_EFR,
	.codec_type		= CODEC_EFR,
	.name			= "racal-efr",
	.description		= "Racal EFR TCH/F recording",

	.frame_len		= RACAL_EFR_LEN,
	.conv_from_canon	= racal_efr_from_canon,
	.conv_to_canon		= racal_efr_to_canon,
};
