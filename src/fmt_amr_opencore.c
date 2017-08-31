/* Input format to libopencore-amrnb: Exactly like our canonical AMR */
/* (C) 2017 Harald Welte <laforge@gnumonks.org> */

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

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/utils.h>

static int
amr_opencore_from_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	memcpy(dst, src, src_len);
	return src_len;
}

static int
amr_opencore_to_canon(uint8_t *dst, const uint8_t *src, unsigned int src_len)
{
	memcpy(dst, src, src_len);
	return src_len;
}

const struct osmo_gapk_format_desc fmt_amr_opencore = {
	.type			= FMT_AMR_OPENCORE,
	.codec_type		= CODEC_AMR,
	.name			= "amr-opencore",
	.description		= "Input format to libopencore-amrnb",

	.frame_len		= 0,
	.conv_from_canon	= amr_opencore_from_canon,
	.conv_to_canon		= amr_opencore_to_canon,
};
