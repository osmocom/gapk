/* HR RTP Payload according to ETSI TS 101 318. Incompatible with RFC5993! */
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

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/utils.h>

/* conversion function: RTP payload -> canonical format */
static int
rtp_hr_etsi_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	/* according to TS 101 318 */
	assert(src_len == HR_CANON_LEN);
	memcpy(dst, src, src_len);

	return src_len;
}

/* conversion function: canonical format -> RTP payload */
static int
rtp_hr_etsi_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	/* according to TS 101 318 */
	assert(src_len == HR_CANON_LEN);
	memcpy(dst, src, src_len);

	return HR_CANON_LEN;
}

const struct format_desc fmt_rtp_hr_etsi = {
	.type			= FMT_RTP_HR_ETSI,
	.codec_type		= CODEC_HR,
	.name			= "rtp-hr-etsi",
	.description		= "RTP payload for HR according to ETSI TS 101 318",

	.frame_len		= HR_CANON_LEN,
	.conv_from_canon	= rtp_hr_etsi_from_canon,
	.conv_to_canon		= rtp_hr_etsi_to_canon,
};
