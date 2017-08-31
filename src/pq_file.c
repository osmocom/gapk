/* Process Queue: File handling tasks */

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
#include <stdio.h>
#include <stdlib.h>

#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/procqueue.h>


struct pq_state_file {
	FILE *fh;
	unsigned int blk_len;
};


static int
pq_cb_file_input(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	struct pq_state_file *state = _state;
	int rv;
	rv = fread(out, state->blk_len, 1, state->fh);
	if (rv <= 0)
		return -1;
	return rv * state->blk_len;
}

static int
pq_cb_file_output(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	struct pq_state_file *state = _state;
	int rv;
	rv = fwrite(in, in_len, 1, state->fh);
	return rv == 1 ? 0 : -1;
}

static void
pq_cb_file_exit(void *_state)
{
	free(_state);
}

static int
pq_queue_file_op(struct osmo_gapk_pq *pq, FILE *fh, unsigned int blk_len, int in_out_n)
{
	struct osmo_gapk_pq_item *item;
	struct pq_state_file *state;

	state = calloc(1, sizeof(struct pq_state_file));
	if (!state)
		return -ENOMEM;

	state->fh = fh;
	state->blk_len = blk_len;

	item = osmo_gapk_pq_add_item(pq);
	if (!item) {
		free(state);
		return -ENOMEM;
	}

	item->len_in  = in_out_n ? 0 : blk_len;
	item->len_out = in_out_n ? blk_len : 0;
	item->state   = state;
	item->proc    = in_out_n ? pq_cb_file_input : pq_cb_file_output;
	item->exit    = pq_cb_file_exit;

	return 0;
}


/*! Add file input to given processing queue
 *  This usually only makes sense as first item in the queue
 *  \param pq Processing Queue to add the input file to
 *  \param[in] src caller-fopen()ed input file
 *  \param[in] blk_len block length to be read from file
 *  \returns 0 on sucess; negative on error */
int
osmo_gapk_pq_queue_file_input(struct osmo_gapk_pq *pq, FILE *src, unsigned int blk_len)
{
	fprintf(stderr, "[+] PQ: Adding file input (blk_len=%u)\n", blk_len);
	return pq_queue_file_op(pq, src, blk_len, 1);
}

/*! Add file output to given processing queue
 *  This usually only makes sense as first item in the queue
 *  \param pq Processing Queue to add the output file to
 *  \param[in] dst caller-fopen()ed output file
 *  \param[in] blk_len block length to be written to file
 *  \returns 0 on sucess; negative on error */
int
osmo_gapk_pq_queue_file_output(struct osmo_gapk_pq *pq, FILE *dst, unsigned int blk_len)
{
	fprintf(stderr, "[+] PQ: Adding file output (blk_len=%u)\n", blk_len);
	return pq_queue_file_op(pq, dst, blk_len, 0);
}
