/* Null codec: Raw PCM data (16 bits signed) */

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

#include <osmocom/gapk/codecs.h>

const struct osmo_gapk_codec_desc codec_pcm_desc = {
	.type = CODEC_PCM,
	.name = "pcm",
	.description = "Raw PCM signed 16 bits samples",
	.canon_frame_len = PCM_CANON_LEN,
};
