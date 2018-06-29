/* ECU (Error Concealment Unit) */

/*
 * This file is part of GAPK (GSM Audio Pocket Knife).
 *
 * (C) 2018 by Vadim Yanitskiy <axilirator@gmail.com>
 *
 * All Rights Reserved
 *
 * GAPK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GAPK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GAPK.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <talloc.h>
#include <errno.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/logging.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/codecs.h>

#include <osmocom/codec/ecu.h>

static void
clean_up(void *state)
{
	talloc_free(state);
}

/*! Add a ECU block to the processing queue
 *  (To be placed between frame source and decoder)
 *  \param pq Processing Queue to which the block is added
 *  \returns 0 on success; negative on error */
int
osmo_gapk_pq_queue_ecu(struct osmo_gapk_pq *pq,
	const struct osmo_gapk_codec_desc *codec)
{
	const struct osmo_gapk_format_desc *fmt;
	struct osmo_gapk_pq_item *item;

	/* Make sure that a codec has corresponding ECU implementation */
	if (codec->ecu_proc == NULL) {
		LOGPGAPK(LOGL_ERROR, "Codec '%s' has no ECU implementation\n", codec->name);
		return -ENOTSUP;
	}

	/* Allocate a new item to the processing queue */
	item = osmo_gapk_pq_add_item(pq);
	if (!item)
		return -ENOMEM;

	/* Allocate the ECU state */
	item->state = talloc_zero(item, struct osmo_ecu_fr_state);
	if (!item->state) {
		talloc_free(item);
		return -ENOMEM;
	}

	/* I/O signature shall match the input signature of a codec */
	fmt = osmo_gapk_fmt_get_from_type(codec->codec_dec_format_type);
	item->len_in  = fmt->frame_len;
	item->len_out = fmt->frame_len;

	item->proc = codec->ecu_proc;
	item->exit = &clean_up;
	item->wait = NULL;

	/* Meta information */
	item->type = OSMO_GAPK_ITEM_TYPE_PROC;
	item->cat_name = "ecu";
	item->sub_name = codec->name;

	LOGPGAPK(LOGL_DEBUG, "PQ '%s': Adding ECU for codec '%s', "
		"format '%s'\n", pq->name, codec->name, fmt->name);

	return 0;
}
