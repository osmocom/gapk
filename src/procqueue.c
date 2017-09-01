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

#include <osmocom/core/linuxlist.h>

#include <osmocom/gapk/procqueue.h>

/* crate a new (empty) processing queue */
struct osmo_gapk_pq *
osmo_gapk_pq_create(void)
{
	struct osmo_gapk_pq *pq;

	/* Allocate memory for a new processing queue */
	pq = (struct osmo_gapk_pq *) calloc(1, sizeof(struct osmo_gapk_pq));

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
		free(item->buf);

		/* Call exit handler if preset */
		if (item->exit)
			item->exit(item->state);

		/* Delete an item from list */
		llist_del(&item->list);
		free(item);
	}

	free(pq);
}

/*! allocate + add an item to a processing queue; return new item
 *  \param[in] pq Processing Queue to which item is added
 *  \returns new PQ item; NULL on error */
struct osmo_gapk_pq_item *
osmo_gapk_pq_add_item(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;

	/* Allocate memory for a new item */
	item = calloc(1, sizeof(struct osmo_gapk_pq_item));
	if (!item)
		return NULL;

	/* Add one to the end of a queue */
	llist_add_tail(&item->list, &pq->items);

	/* Increase the items count */
	pq->n_items++;

	return item;
}

/*! prepare a processing queue; allocates buffers; checks lengths
 *  \param[in] pq Processing Queue to be prepared
 *  \returns 0 on succcess; negative on error */
int
osmo_gapk_pq_prepare(struct osmo_gapk_pq *pq)
{
	struct osmo_gapk_pq_item *item;
	unsigned int len_prev = 0;

	/* Iterate over all items in queue */
	llist_for_each_entry(item, &pq->items, list) {
		/* Make sure I/O data lengths are equal */
		if (item->len_in && item->len_in != len_prev) {
			fprintf(stderr, "[!] PQ item requires input size %u, "
				"but previous output is %u\n", item->len_in, len_prev);
			return -EINVAL;
		}

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
			item->buf = malloc(buf_size);
			if (!item->buf)
				return -ENOMEM;
		} else {
			/* Make sure the last item is a sink */
			if (item->len_out)
				return -EINVAL;
		}

		/* Store output length for further comparation */
		len_prev = item->len_out;
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
			fprintf(stderr, "[!] osmo_gapk_pq_execute(): "
				"abort, item returned %d\n", rv);
			return rv;
		}

		buf_prev = item->buf;
		len_prev = rv;
	}

	return 0;
}
