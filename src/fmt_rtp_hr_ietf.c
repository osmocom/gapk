/* HR RTP Payload according to RFC5993. Incompatible with ETSI TS 101 318! */
/* (C) 2017 by Harald Welte <laforge@gnumonks.org> */

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
#include <string.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/utils.h>

#define HR_LEN	(HR_CANON_LEN+1)

/* See Section 5.2 of RFC5993 */
enum rtp_hr_ietf_ft {
	FT_GOOD_SPEECH	= 0,
	FT_GOOD_SID	= 2,
	FT_NO_DATA	= 7,
};

/* conversion function: RTP payload -> canonical format */
static int
rtp_hr_ietf_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	enum rtp_hr_ietf_ft ft;

	/* according to RFC5993 */
	assert(src_len == HR_CANON_LEN);

	/* TODO: Determine SID frames / NO_DATA frames */
	ft = FT_GOOD_SPEECH;
	/* Add ToC byte */
	dst[0] = ((ft & 0x7) << 4);

	/* copy codec payload */
	memcpy(dst+1, src, src_len);

	return HR_CANON_LEN;
}

/* conversion function: canonical format -> RTP payload */
static int
rtp_hr_ietf_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	/* according to RFC5993 */
	assert(src_len == HR_LEN);

	/* Remove ToC byte */
	memcpy(dst, src+1, HR_CANON_LEN);

	return HR_CANON_LEN;
}

const struct format_desc fmt_rtp_hr_ietf = {
	.type			= FMT_RTP_HR_IETF,
	.codec_type		= CODEC_HR,
	.name			= "rtp-hr-ietf",
	.description		= "RTP payload for HR according to IETF RFC5993",

	.frame_len		= HR_LEN,
	.conv_from_canon	= rtp_hr_ietf_from_canon,
	.conv_to_canon		= rtp_hr_ietf_to_canon,
};
