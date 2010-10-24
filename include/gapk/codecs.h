/* Codecs handling */

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

#ifndef __GAPK_CODECS_H__
#define __GAPK_CODECS_H__

enum codec_type {
	CODEC_INVALID = 0,
	CODEC_PCM,	/* 16 bits PCM samples */
	CODEC_HR,	/* GSM Half Rate codec GSM 06.20 */
	CODEC_FR,	/* GSM Full Rate codec GSM 06.10 */
	CODEC_EFR,	/* GSM Enhanced Full Rate codec GSM 06.60 */
	_CODEC_MAX,
};

struct codec_desc {
	enum codec_type		type;
	const char *		name;
	const char *		description;
	unsigned int		canon_frame_len;
};

const struct codec_desc *codec_get_from_type(enum codec_type type);

#endif /* __GAPK_CODECS_H__ */
