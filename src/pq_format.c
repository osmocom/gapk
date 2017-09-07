/* Process Queue: Format handling tasks */

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

#include <errno.h>
#include <stdint.h>

#include <osmocom/gapk/logging.h>
#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/procqueue.h>


static int
pq_cb_fmt_convert(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	osmo_gapk_fmt_conv_cb_t f = _state;
	return f(out, in, in_len);
}

/*! Add format conversion to processing queue
 *  \param pq Processing Queue to add conversion to
 *  \param[in] fmt Format description for conversion
 *  \param[in] to_from_n convert to (0) or from (1) specified format */
int
osmo_gapk_pq_queue_fmt_convert(struct osmo_gapk_pq *pq, const struct osmo_gapk_format_desc *fmt, int to_from_n)
{
	const struct osmo_gapk_codec_desc *codec;
	struct osmo_gapk_pq_item *item;

	codec = osmo_gapk_codec_get_from_type(fmt->codec_type);
	if (!codec) {
		LOGPGAPK(LOGL_ERROR, "Cannot determine codec from "
			"format %s\n", fmt->name);
		return -EINVAL;
	}


	item = osmo_gapk_pq_add_item(pq);
	if (!item)
		return -ENOMEM;

	if (to_from_n) {
		LOGPGAPK(LOGL_DEBUG, "PQ: Adding conversion from canon "
			"to %s (for codec %s)\n", fmt->name, codec->name);
		item->len_in  = codec->canon_frame_len;
		item->len_out = fmt->frame_len;
		item->state   = fmt->conv_from_canon;
	} else {
		LOGPGAPK(LOGL_DEBUG, "PQ: Adding conversion from %s "
			"to canon (for codec %s)\n", fmt->name, codec->name);
		item->len_in  = fmt->frame_len;
		item->len_out = codec->canon_frame_len;
		item->state   = fmt->conv_to_canon;
	}

	item->proc = pq_cb_fmt_convert;
	item->wait = NULL;

	return 0;
}
