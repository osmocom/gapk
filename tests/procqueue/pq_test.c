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
#include <talloc.h>
#include <assert.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/common.h>

/**
 * This test is intended to validate the processing queue
 * management API. Moreover, the talloc debugging API is
 * used to ensure that there are no memory leaks.
 *
 * First, four processing queues are being allocated. One
 * of them is empty, while others have different count of
 * items. Then the human-readable description is being
 * generated for all of them. And finally, the processing
 * and exit callback are being tested.
 *
 * During the test execution, the talloc NULL-context
 * tracking feature is enabled, allowing to observe every
 * memory allocation within the libosmogapk, and to detect
 * memory leaks.
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

static int q3_i0_proc(void *state, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	int i;

	printf("Incoming data: ");

	for (i = 0; i < 10; i++) {
		printf("%d ", i);
		out[i] = i;
	}

	printf("\n");

	return 0;
}

static int q3_i1_proc(void *state, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	int i;

	for (i = 0; i < 10; i++)
		out[i] = in[i] % 2;

	return 0;
}

static int q3_i2_proc(void *state, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	int i;

	printf("Outgoing data: ");

	for (i = 0; i < 10; i++)
		printf("%d ", in[i]);

	printf("\n");

	return 0;
}

static void q3_exit(void *state)
{
	struct osmo_gapk_pq_item *item;

	/* Make sure the item's state is passed correctly */
	assert(state != NULL);

	item = (struct osmo_gapk_pq_item *) state;
	printf("Calling exit callback for '%s/%s'\n",
		item->cat_name, item->sub_name);
}

int main(int argc, char **argv)
{
	/* Enable tracking the use of NULL memory contexts */
	talloc_enable_null_tracking();

	/**
	 * 1. Processing queue allocation test
	 */
	struct osmo_gapk_pq *q0, *q1, *q2, *q3;

	printf("Processing queue allocation test:\n");

	/* Allocate four queues */
	q0 = osmo_gapk_pq_create(NULL);
	q1 = osmo_gapk_pq_create("q1");
	q2 = osmo_gapk_pq_create("q2");
	q3 = osmo_gapk_pq_create("q3");

	/* Make sure all queues are allocated */
	assert(q0 != NULL);
	assert(q1 != NULL);
	assert(q2 != NULL);
	assert(q3 != NULL);

	/* Print talloc memory hierarchy */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);
	printf("\n");


	/**
	 * 2. Item allocation test
	 */
	struct osmo_gapk_pq_item *q3_i0, *q3_i1, *q3_i2;
	struct osmo_gapk_pq_item *q2_i0, *q2_i1;
	struct osmo_gapk_pq_item *q1_i0;

	printf("Item allocation test:\n");

	/* Make sure there are no items */
	assert(q0->n_items == 0);
	assert(q1->n_items == 0);
	assert(q2->n_items == 0);
	assert(q3->n_items == 0);

	/* Allocate items */
	q3_i0 = osmo_gapk_pq_add_item(q3);
	q3_i1 = osmo_gapk_pq_add_item(q3);
	q3_i2 = osmo_gapk_pq_add_item(q3);
	q2_i0 = osmo_gapk_pq_add_item(q2);
	q2_i1 = osmo_gapk_pq_add_item(q2);
	q1_i0 = osmo_gapk_pq_add_item(q1);

	/* Make sure all items are allocated */
	assert(q3_i0 != NULL);
	assert(q3_i1 != NULL);
	assert(q3_i2 != NULL);
	assert(q2_i0 != NULL);
	assert(q2_i1 != NULL);
	assert(q1_i0 != NULL);

	/* Check item count */
	assert(q0->n_items == 0);
	assert(q1->n_items == 1);
	assert(q2->n_items == 2);
	assert(q3->n_items == 3);

	/* Print talloc memory hierarchy */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);
	printf("\n");


	/**
	 * 3. Items persistence test
	 */
	struct osmo_gapk_pq_item *item;

	/* Make sure that q0 is empty, others are not */
	assert(llist_empty(&q0->items) == 1);
	assert(llist_empty(&q1->items) == 0);
	assert(llist_empty(&q2->items) == 0);
	assert(llist_empty(&q3->items) == 0);

	/* A single item must be the first and the last in a queue */
	item = llist_first_entry(&q1->items, struct osmo_gapk_pq_item, list);
	assert(item == q1_i0);
	item = llist_last_entry(&q1->items, struct osmo_gapk_pq_item, list);
	assert(item == q1_i0);

	/* Two items: one is the first, second is the last */
	item = llist_first_entry(&q2->items, struct osmo_gapk_pq_item, list);
	assert(item == q2_i0);
	item = llist_last_entry(&q2->items, struct osmo_gapk_pq_item, list);
	assert(item == q2_i1);

	/* Three items: one is the first, third is the last */
	item = llist_first_entry(&q3->items, struct osmo_gapk_pq_item, list);
	assert(item == q3_i0);
	assert(item != q3_i1);
	item = llist_last_entry(&q3->items, struct osmo_gapk_pq_item, list);
	assert(item == q3_i2);
	assert(item != q3_i1);


	/**
	 * 4. Queue I/O data lengths check
	 */
	q3_i0->len_in = 10;
	q3_i0->len_out = 20;

	q3_i1->len_in = 20;
	q3_i1->len_out = 30;

	q3_i2->len_in = 30;
	q3_i2->len_out = 10;

	/* Normal case */
	assert(osmo_gapk_pq_check(q3, 0) == 0);

	/* Abnormal case (I/O length mismatch) */
	q3_i0->len_out = 10;
	assert(osmo_gapk_pq_check(q3, 0) != 0);
	q3_i0->len_out = 20;

	/* Check queue in strict mode (requires src -> ... -> sink) */
	q3_i0->type = OSMO_GAPK_ITEM_TYPE_SOURCE;
	q3_i1->type = OSMO_GAPK_ITEM_TYPE_PROC;
	q3_i2->type = OSMO_GAPK_ITEM_TYPE_SINK;

	/* Normal case (src -> proc -> sink) */
	assert(osmo_gapk_pq_check(q3, 1) == 0);

	/* Abnormal case (proc -> proc -> sink) */
	q3_i0->type = OSMO_GAPK_ITEM_TYPE_PROC;
	assert(osmo_gapk_pq_check(q3, 1) != 0);
	q3_i0->type = OSMO_GAPK_ITEM_TYPE_SOURCE;


	/**
	 * 5. Buffer allocation test
	 */
	printf("Queue preparation test:\n");

	/* Prepare the queue */
	assert(osmo_gapk_pq_prepare(q3) == 0);

	/* Make sure buffers were allocated */
	assert(q3_i0->buf != NULL);
	assert(q3_i1->buf != NULL);

	/* Currently, we don't allocate buffers for sinks */
	assert(q3_i2->buf == NULL);

	/* Compare required vs allocated buffer sizes */
	assert(talloc_total_size(q3_i0->buf) == q3_i0->len_out);
	assert(talloc_total_size(q3_i1->buf) == q3_i1->len_out);

	/* Print talloc memory hierarchy */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);
	printf("\n");


	/**
	 * 6. Queue description test
	 */
	char *queue_desc;

	printf("Queue description test:\n");

	/* An empty queue doesn't have description */
	queue_desc = osmo_gapk_pq_describe(q0);
	assert(queue_desc == NULL);

	/* Fill in some data */
	q3_i0->cat_name = "source";
	q3_i0->sub_name = "file";
	q3_i1->cat_name = "proc";
	q3_i1->sub_name = "dummy";
	q3_i2->cat_name = "sink";
	q3_i2->sub_name = "alsa";

	q2_i0->cat_name = "source";
	q2_i0->sub_name = "dummy";
	q2_i1->cat_name = "sink";
	q2_i1->sub_name = "dummy";

	q1_i0->cat_name = "dummy";
	q1_i0->sub_name = "dummy";

	queue_desc = osmo_gapk_pq_describe(q3);
	assert(queue_desc != NULL);
	printf("Queue q3 description: %s\n", queue_desc);
	talloc_free(queue_desc);

	queue_desc = osmo_gapk_pq_describe(q2);
	assert(queue_desc != NULL);
	printf("Queue q2 description: %s\n", queue_desc);
	talloc_free(queue_desc);

	queue_desc = osmo_gapk_pq_describe(q1);
	assert(queue_desc != NULL);
	printf("Queue q1 description: %s\n\n", queue_desc);
	talloc_free(queue_desc);


	/**
	 * 7. Queue execution test
	 */
	printf("Processing queue execution test:\n");

	/* Make sure there are no callbacks by default */
	assert(q3_i0->proc == NULL);
	assert(q3_i0->wait == NULL);
	assert(q3_i0->exit == NULL);

	assert(q3_i1->proc == NULL);
	assert(q3_i1->wait == NULL);
	assert(q3_i1->exit == NULL);

	assert(q3_i2->proc == NULL);
	assert(q3_i2->wait == NULL);
	assert(q3_i2->exit == NULL);

	q3_i0->proc = &q3_i0_proc;
	q3_i1->proc = &q3_i1_proc;
	q3_i2->proc = &q3_i2_proc;

	assert(osmo_gapk_pq_execute(q3) == 0);
	printf("\n");


	/**
	 * 8. Exit callback & deallocation tests
	 */
	printf("Processing queue exit callback test:\n");

	q3_i0->state = q3_i0;
	q3_i1->state = q3_i1;
	q3_i2->state = q3_i2;

	q3_i0->exit = &q3_exit;
	q3_i1->exit = &q3_exit;
	q3_i2->exit = &q3_exit;

	osmo_gapk_pq_destroy(q0);
	osmo_gapk_pq_destroy(q1);
	osmo_gapk_pq_destroy(q2);
	osmo_gapk_pq_destroy(q3);

	printf("\n");


	/**
	 * 9. Memory leak detection test
	 */
	printf("Processing queue deallocation test:\n");
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);

	return 0;
}
