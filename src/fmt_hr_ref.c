/* 3GPP Reference HR vocoder file formats 
 *
 * See http://ftp.3gpp.org/Specs/2001-06/Ph2/06_series/0606-421/0606_421.zip
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

#include <assert.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/utils.h>

#define HR_REF_DEC_LEN	(22 * sizeof(uint16_t))
#define HR_REF_ENC_LEN	(20 * sizeof(uint16_t))

static const int params_unvoiced[] = {
	5,	/* R0 */
	11,	/* k1Tok3 */
	9,	/* k4Tok6 */
	8,	/* k7Tok10 */
	1,	/* softInterpolation */
	2,	/* voicingDecision */
	7,	/* code1_1 */
	7,	/* code2_1 */
	5,	/* gsp0_1 */
	7,	/* code1_2 */
	7,	/* code2_2 */
	5,	/* gsp0_2 */
	7,	/* code1_3 */
	7,	/* code2_3 */
	5,	/* gsp0_3 */
	7,	/* code1_4 */
	7,	/* code2_4 */
	5,	/* gsp0_4 */
};

static const int params_voiced[] = {
	5,	/* R0 */
	11,	/* k1Tok3 */
	9,	/* k4Tok6 */
	8,	/* k7Tok10 */
	1,	/* softInterpolation */
	2,	/* voicingDecision */
	8,	/* frameLag */
	9,	/* code_1 */
	5,	/* gsp0_1 */
	4,	/* deltaLag_2 */
	9,	/* code_2 */
	5,	/* gsp0_2 */
	4,	/* deltaLag_3 */
	9,	/* code_3 */
	5,	/* gsp0_3 */
	4,	/* deltaLag_4 */
	9,	/* code_4 */
	5,	/* gsp0_4 */
};


static int
hr_ref_from_canon(uint16_t *hr_ref, const uint8_t *canon)
{
	int i, j, voiced;
	const int *params;

	voiced = (msb_get_bit(canon, 34) << 1) | msb_get_bit(canon, 35);
	params = voiced ? &params_voiced[0] : &params_unvoiced[0];

	for (i=0,j=0; i<18; i++)
	{
		uint16_t w;
		int l, k;

		l = params[i];

		w = 0;
		for (k=0; k<l; k++)
			w = (w << 1) | msb_get_bit(canon, j+k);
		hr_ref[i] = w;

		j += l;
	}

	return 0;
}

static int
hr_ref_to_canon(uint8_t *canon, const uint16_t *hr_ref)
{
	int i, j, voiced;
	const int *params;

	voiced = hr_ref[5]; /* voicingDecision */
	params = voiced ? &params_voiced[0] : &params_unvoiced[0];

	for (i=0,j=0; i<18; i++)
	{
		uint16_t w;
		int l, k;

		l = params[i];

		w = hr_ref[i];
		for (k=0; k<l; k++)
			msb_put_bit(canon, j+k, w & (1<<(l-k-1)));

		j += l;
	}

	return 0;
}

static int
hr_ref_dec_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	uint16_t *hr_ref = (uint16_t *)dst;
	int rv;

	assert(src_len == HR_CANON_LEN);

	rv = hr_ref_from_canon(hr_ref, src);
	if (rv)
		return rv;

	hr_ref[18] = 0;		/* BFI : 1 bit */
	hr_ref[19] = 0;		/* UFI : 1 bit */
	hr_ref[20] = 0;		/* SID : 2 bit */
	hr_ref[21] = 0;		/* TAF : 1 bit */

	return HR_REF_DEC_LEN;
}

static int
hr_ref_dec_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	const uint16_t *hr_ref = (const uint16_t *)src;
	int rv;

	assert(src_len == HR_REF_DEC_LEN);

	rv = hr_ref_to_canon(dst, hr_ref);
	if (rv)
		return rv;

	return HR_CANON_LEN;
}

static int
hr_ref_enc_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	uint16_t *hr_ref = (uint16_t *)dst;
	int rv;

	assert(src_len == HR_CANON_LEN);

	rv = hr_ref_from_canon(hr_ref, src);
	if (rv)
		return rv;

	hr_ref[18] = 0;		/* SP  : 1 bit */
	hr_ref[19] = 0;		/* VAD : 1 bit */

	return HR_REF_ENC_LEN;
}

static int
hr_ref_enc_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	const uint16_t *hr_ref = (const uint16_t *)src;
	int rv;

	assert(src_len == HR_REF_ENC_LEN);

	rv = hr_ref_to_canon(dst, hr_ref);
	if (rv)
		return rv;

	return HR_CANON_LEN;
}


const struct format_desc fmt_hr_ref_dec = {
	.type			= FMT_HR_REF_DEC,
	.codec_type		= CODEC_HR,
	.name			= "hr-ref-dec",
	.description		= "3GPP HR Reference decoder code parameters file format",

	.frame_len		= HR_REF_DEC_LEN,
	.conv_from_canon	= hr_ref_dec_from_canon,
	.conv_to_canon		= hr_ref_dec_to_canon,
};

const struct format_desc fmt_hr_ref_enc = {
	.type			= FMT_HR_REF_ENC,
	.codec_type		= CODEC_HR,
	.name			= "hr-ref-enc",
	.description		= "3GPP HR Reference encoder code parameters file format",

	.frame_len		= HR_REF_ENC_LEN,
	.conv_from_canon	= hr_ref_enc_from_canon,
	.conv_to_canon		= hr_ref_enc_to_canon,
};
