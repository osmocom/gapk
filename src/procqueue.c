/* Processing Queue Management */

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
#include <stdlib.h>
#include <talloc.h>

#include <osmocom/core/linuxlist.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/logging.h>

/* Internal root talloc context */
extern TALLOC_CTX *gapk_root_ctx;

/* crate a new (empty) processing queue */
struct osmo_gapk_pq *
osmo_gapk_pq_create(const char *name)
{
	struct osmo_gapk_pq *pq;

	/* Allocate memory for a new processing queue */
	pq = talloc_zero(gapk_root_ctx, struct osmo_gapk_pq);
	if (!pq)
		return NULL;

	if (name != NULL) {
		/* Rename talloc context */
		talloc_set_name(pq, "struct osmo_gapk_pq '%s'", name);
		/* Set queue name */
		pq->name = name;
	}

	/* Init its list of items */
	INIT_LLIST_HEAD(&pq->items);

	return pq;
}

/*! destroy a processing queue, calls exit() callback of each item
 *  \param[in] pq Processing Queue to be destroyed */
void
osmo_gapk_pq_destroy(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item, *item_next;

	if (!pq)
		return;

	/* Iterate over all items in queue */
	llist_for_each_entry_safe(item, item_next, &pq->items, list) {
		/* Free output buffer memory */
		talloc_free(item->buf);

		/* Call exit handler if preset */
		if (item->exit)
			item->exit(item->state);

		/* Delete an item from list */
		llist_del(&item->list);
		talloc_free(item);
	}

	talloc_free(pq);
}

/*! allocate + add an item to a processing queue; return new item
 *  \param[in] pq Processing Queue to which item is added
 *  \returns new PQ item; NULL on error */
struct osmo_gapk_pq_item *
osmo_gapk_pq_add_item(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;

	/* Allocate memory for a new item */
	item = talloc_zero(pq, struct osmo_gapk_pq_item);
	if (!item)
		return NULL;

	/* Add one to the end of a queue */
	llist_add_tail(&item->list, &pq->items);

	/* Increase the items count */
	pq->n_items++;

	return item;
}

/*! check a processing queue; make sure I/O data lengths are equal
 *  \param[in] pq     Make sure both source and sink are preset
 *  \param[in] strict Processing Queue to be checked
 *  \returns 0 on succcess; negative on error */
int
osmo_gapk_pq_check(struct osmo_gapk_pq *pq, int strict)
{
	struct osmo_gapk_pq_item *item_prev = NULL;
	struct osmo_gapk_pq_item *item;

	/* Make sure I/O data lengths are equal */
	llist_for_each_entry(item, &pq->items, list) {
		if (item_prev && item->len_in) {
			if (item->len_in != item_prev->len_out) {
				LOGPGAPK(LOGL_ERROR, "PQ '%s': item '%s/%s' requires "
					"input size %u, but previous '%s/%s' has %u\n",
					pq->name, item->cat_name, item->sub_name,
					item->len_in, item_prev->cat_name,
					item_prev->sub_name, item_prev->len_out);
				return -EINVAL;
			}
		}

		/* Save pointer to the previous item */
		item_prev = item;
	}

	if (strict) {
		/* Make sure the first item is a source */
		item = llist_first_entry(&pq->items,
			struct osmo_gapk_pq_item, list);
		if (item->type != OSMO_GAPK_ITEM_TYPE_SOURCE)
			goto src_sink_err;

		/* Make sure the last item is a sink */
		item = llist_last_entry(&pq->items,
			struct osmo_gapk_pq_item, list);
		if (item->type != OSMO_GAPK_ITEM_TYPE_SINK)
			goto src_sink_err;
	}

	return 0;

src_sink_err:
	LOGPGAPK(LOGL_ERROR, "PQ '%s': the first item should be a source, "
		"and the last one should be a sink\n", pq->name);
	return -EINVAL;
}

/*! prepare a processing queue; allocates buffers
 *  \param[in] pq Processing Queue to be prepared
 *  \returns 0 on succcess; negative on error */
int
osmo_gapk_pq_prepare(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;

	/* Iterate over all items in queue */
	llist_for_each_entry(item, &pq->items, list) {
		/* The sink item doesn't require an output buffer */
		if (item->list.next != &pq->items) {
			unsigned int buf_size = item->len_out;

			/**
			 * Use maximum known buffer size
			 * for variable-length codec output
			 */
			if (!buf_size)
				buf_size = VAR_BUF_SIZE;

			/* Allocate memory for an output buffer */
			item->buf = talloc_size(item, buf_size);
			if (!item->buf)
				return -ENOMEM;
		}
	}

	return 0;
}

/*! execute a processing queue; iterate over processing elements
 *  \param[in] pq Processing Queue to be executed
 *  \returns 0 on success; negative on error (if any item returns negative) */
int
osmo_gapk_pq_execute(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;
	unsigned int len_prev = 0;
	uint8_t *buf_prev = NULL;
	int rv;

	/* Iterate over all items in queue */
	llist_for_each_entry(item, &pq->items, list) {
		/* Call item's processing handler */
		rv = item->proc(item->state, item->buf, buf_prev, len_prev);
		if (rv < 0) {
			LOGPGAPK(LOGL_ERROR, "Queue execution aborted: "
				"item returned %d\n", rv);
			return rv;
		}

		buf_prev = item->buf;
		len_prev = rv;
	}

	return 0;
}

char *
osmo_gapk_pq_describe(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;
	char *result = NULL;
	int i = 0;

	/* Iterate over all items in queue */
	llist_for_each_entry(item, &pq->items, list) {
		result = talloc_asprintf_append(result, "%s/%s%s",
			item->cat_name, item->sub_name,
			++i < pq->n_items ? " -> " : "");
	}

	return result;
}
