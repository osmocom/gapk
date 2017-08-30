/* AMR RTP Payload according to RFC4867. Only one codec frame per RTP */
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
#include <string.h>

#include <osmocom/codec/codec.h>

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/utils.h>

/* conversion function: RTP payload -> canonical format */
static int
rtp_amr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	/* add Payload Header according to RFC4867 4.4.1 */
	dst[0] = 0xf0;	/* no request */
	memcpy(dst+1, src, src_len);

	return src_len+1;
}

/* conversion function: canonical format -> RTP payload */
static int
rtp_amr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	/* skip Payload Header according to RFC4867 4.4.1 */
	memcpy(dst, src+1, src_len-1);

	return src_len-1;
}

const struct format_desc fmt_rtp_amr = {
	.type			= FMT_RTP_AMR,
	.codec_type		= CODEC_AMR,
	.name			= "rtp-amr",
	.description		= "RTP payload for AMR according to RFC4867",

	.frame_len		= 0,
	.conv_from_canon	= rtp_amr_from_canon,
	.conv_to_canon		= rtp_amr_to_canon,
};
