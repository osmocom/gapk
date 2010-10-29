/* Formats handling */

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

#ifndef __GAPK_FORMATS_H__
#define __GAPK_FORMATS_H__

#include <stdint.h>

enum format_type {
	FMT_INVALID = 0,

	_FMT_MAX,
};

#include <gapk/codecs.h>	/* need to import here because or enum interdep */

typedef int (*fmt_conv_cb_t)(uint8_t *dst, const uint8_t *src);

struct format_desc {
	enum format_type	type;
	enum codec_type		codec_type;
	const char *		name;
	const char *		description;

	unsigned int		frame_len;
	fmt_conv_cb_t		conv_from_canon;
	fmt_conv_cb_t		conv_to_canon;
};

const struct format_desc *fmt_get_from_type(enum format_type type);

#endif /* __GAPK_FORMATS_H__ */
