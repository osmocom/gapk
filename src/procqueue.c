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

#include <osmocom/gapk/procqueue.h>

#define VAR_BUF_SIZE	320
#define MAX_PQ_ITEMS	8

struct pq {
	int		n_items;
	struct pq_item* items[MAX_PQ_ITEMS];
	void *		buffers[MAX_PQ_ITEMS+1];
};


/* crate a new (empty) processing queue */
struct pq *
pq_create(void)
{
	return (struct pq *) calloc(1, sizeof(struct pq));
}

/*! destroy a processing queue, calls exit() callback of each item
 *  \param[in] pq Processing Queue to be destroyed */
void
pq_destroy(struct pq *pq)
{
	int i;

	if (!pq)
		return;

	for (i=0; i<pq->n_items; i++) {
		if (!pq->items[i])
			continue;
		if (pq->items[i]->exit)
			pq->items[i]->exit(pq->items[i]->state);
		free(pq->items[i]);
	}

	for (i=0; i<pq->n_items-1; i++)
		free(pq->buffers[i]); /* free is NULL safe */

	free(pq);
}

/*! allocate + add an item to a processing queue; return new item
 *  \param[in] pq Processing Queue to which item is added
 *  \returns new PQ item; NULL on error */
struct pq_item *
pq_add_item(struct pq *pq)
{
	struct pq_item *item;

	if (pq->n_items == MAX_PQ_ITEMS) {
		fprintf(stderr, "[!] Processing Queue cannot handle more than %u items\n",
			MAX_PQ_ITEMS);
		return NULL;
	}

	item = calloc(1, sizeof(struct pq_item));
	if (!item)
		return NULL;

	pq->items[pq->n_items++] = item;

	return item;
}

/*! prepare a processing queue; allocates buffers; checks lengths
 *  \param[in] pq Processing Queue to be prepared
 *  \returns 0 on succcess; negative on error */
int
pq_prepare(struct pq *pq)
{
	int i;
	unsigned int len_prev;

	len_prev = 0;

	for (i=0; i<pq->n_items; i++) {
		struct pq_item *item = pq->items[i];

		if (item->len_in && item->len_in != len_prev) {
			fprintf(stderr, "[!] PQ item requires input size %u, but previous output is %u\n",
				item->len_in, len_prev);
			return -EINVAL;
		}

		if (i < (pq->n_items-1)) {
			unsigned int buf_size = item->len_out;
			/* variable-length codec output, use maximum
			 * known buffer size */
			if (!buf_size)
				buf_size = VAR_BUF_SIZE;
			pq->buffers[i] = malloc(buf_size);
			if (!pq->buffers[i])
				return -ENOMEM;
		} else{
			if (item->len_out)
				return -EINVAL;
		}

		len_prev = item->len_out;
	}

	return 0;
}

/*! execute a processing queue; iterate over processing elements
 *  \param[in] pq Processing Queue to be executed
 *  \returns 0 on success; negative on error (if any item returns negative) */
int
pq_execute(struct pq *pq)
{
	int i;
	void *buf_prev, *buf;
	unsigned int len_prev;

	buf_prev = NULL;
	len_prev = 0;

	for (i=0; i<pq->n_items; i++) {
		int rv;
		struct pq_item *item = pq->items[i];

		buf = i < (pq->n_items-1) ? pq->buffers[i] : NULL;

		rv = item->proc(item->state, buf, buf_prev, len_prev);
		if (rv < 0) {
			fprintf(stderr, "[!] pq_execute(): abort, item returned %d\n", rv);
			return rv;
		}

		buf_prev = buf;
		len_prev = rv;
	}

	return 0;
}
