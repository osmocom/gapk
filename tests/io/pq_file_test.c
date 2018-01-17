/*
 * This file is part of GAPK (GSM Audio Pocket Knife).
 *
 * (C) 2017 by Vadim Yanitskiy <axilirator@gmail.com>
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

#include <stdio.h>
#include <errno.h>
#include <talloc.h>
#include <assert.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/common.h>

/**
 * This test is intended to check the file source / sink
 * operability. To do that, the following processing chain
 * is being composed:
 *
 *   source/file -> proc/dummy -> sink/file (stdout)
 *
 * The source item opens the sample file named 'io_sample.txt'
 * for reading. The next processing item simply converts all
 * uppercase latters to the lowercase. The last one writes
 * the result to stdout.
 *
 * This processing cycle is being repeated several times
 * with different block length values.
 */

static void talloc_ctx_walk_cb(const void *chunk, int depth,
	int max_depth, int is_ref, void *data)
{
	const char *chunk_name = talloc_get_name(chunk);
	int spaces_cnt;

	/* Hierarchical spacing */
	for (spaces_cnt = 0; spaces_cnt < depth; spaces_cnt++)
		printf("  ");

	/* Chunk info */
	printf("chunk %s: depth=%d\n", chunk_name, depth);
}

static int pq_file_proc(void *state, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	int i;

	/* Change upper case to lower */
	for (i = 0; i < in_len; i++)
		out[i] = (in[i] >= 65 && in[i] <= 90) ?
			in[i] + 32 : in[i];

	return in_len;
}

static int pq_file_check(FILE *src_file, unsigned int blk_len)
{
	struct osmo_gapk_pq_item *proc_item;
	struct osmo_gapk_pq *pq;
	int rc;

	printf("Processing sample file with blk_len=%u:\n", blk_len);

	/* Allocate a processing queue */
	pq = osmo_gapk_pq_create("pq_file_check");
	assert(pq != NULL);

	/* Add a file sink */
	rc = osmo_gapk_pq_queue_file_input(pq, src_file, blk_len);
	assert(rc == 0);

	/* Add a processing item */
	proc_item = osmo_gapk_pq_add_item(pq);
	assert(proc_item != NULL);

	/* Set up I/O parameters */
	proc_item->type = OSMO_GAPK_ITEM_TYPE_PROC;
	proc_item->len_in = blk_len;
	proc_item->len_out = blk_len;

	/* Set the processing callback */
	proc_item->proc = &pq_file_proc;

	/* Add a sink item */
	rc = osmo_gapk_pq_queue_file_output(pq, stdout, blk_len);
	assert(rc == 0);

	/* Check a queue in strict mode */
	rc = osmo_gapk_pq_check(pq, 1);
	assert(rc == 0);

	/* Prepare a queue */
	rc = osmo_gapk_pq_prepare(pq);
	assert(rc == 0);

	while (1) {
		rc = osmo_gapk_pq_execute(pq);
		if (rc)
			break;
	}

	/* Destroy processing queue */
	osmo_gapk_pq_destroy(pq);

	return 0;
}

int main(int argc, char **argv)
{
	int i;

	/* Enable tracking the use of NULL memory contexts */
	talloc_enable_null_tracking();

	/* Open sample file */
	FILE *sample_file = fopen(argv[1], "rb");
	assert(sample_file != NULL);

	/* Process sample file with different blk_len values */
	for (i = 2; i <= 32; i *= 2) {
		pq_file_check(sample_file, i);
		rewind(sample_file);
	}

	printf("\n");

	/* Close sample file */
	fclose(sample_file);

	/* Print talloc memory hierarchy */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);

	return 0;
}
