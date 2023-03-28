/* EFR RTP Payload according to RFC3551. Only one codec frame per RTP */
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

#include <osmocom/codec/codec.h>

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/utils.h>

#define EFR_LEN	  31
#define EFR_MAGIC 0xc

/* EFR encoding starts with 0xc, if all other bits are 0 the packet is probably damaged */
static const uint8_t DAMAGED_PACKET[EFR_LEN] = {
	0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* EFR SID frame is according to TS 101 318 5.3.2 */
static const uint8_t SILENCE[EFR_LEN] = {
	0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* conversion function: RTP payload -> canonical format */
static int
rtp_efr_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	/* according to RFC3551 4.5.9 */
	assert(src_len == EFR_CANON_LEN);

	dst[0] = (EFR_MAGIC << 4) | (src[0] >> 4);
	for (i=1; i<EFR_LEN; i++)
		dst[i] = (src[i-1] << 4) | (src[i] >> 4);

	return EFR_LEN;
}

/* conversion function: canonical format -> RTP payload */
static int
rtp_efr_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	int i;

	/* broken RTP frames may be short; substitute empty frame */
	if (src_len != EFR_LEN) {
		memset(dst, 0, EFR_CANON_LEN);
		return EFR_CANON_LEN;
	}

	if (!memcmp(DAMAGED_PACKET, src, EFR_LEN))
		src = SILENCE;

	for (i=0; i<(EFR_LEN-1); i++)
		dst[i] = (src[i] << 4) | (src[i+1] >> 4);
	dst[EFR_LEN-1] = src[EFR_LEN-1] << 4;

	return EFR_CANON_LEN;
}

const struct osmo_gapk_format_desc fmt_rtp_efr = {
	.type			= FMT_RTP_EFR,
	.codec_type		= CODEC_EFR,
	.name			= "rtp-efr",
	.description		= "RTP payload for EFR according to RFC3551",

	.frame_len		= EFR_LEN,
	.conv_from_canon	= rtp_efr_from_canon,
	.conv_to_canon		= rtp_efr_to_canon,
};
