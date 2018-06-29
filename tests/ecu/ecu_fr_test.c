/*
 * This file is part of GAPK (GSM Audio Pocket Knife).
 *
 * (C) 2018 by Vadim Yanitskiy <axilirator@gmail.com>
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
#include <string.h>
#include <assert.h>

#include <osmocom/core/utils.h>
#include <osmocom/codec/codec.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/common.h>

/* A good FR frame */
static const char *sample_frame_hex = \
	"d9ec9be212901f802335598c501f805bad3d4ba01f809b69df5a501f809cd1b4da";

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

void pq_execute(struct osmo_gapk_pq *pq, uint8_t *frame, size_t len)
{
	struct osmo_gapk_pq_item *pq_item;
	unsigned int len_prev = len;
	uint8_t *buf_prev = frame;
	int rv;

	/* Iterate over all items in the chain */
	llist_for_each_entry(pq_item, &pq->items, list) {
		printf("Block '%s/%s/%s' in (len=%d): %s\n", pq->name,
			pq_item->cat_name, pq_item->sub_name, len_prev,
			osmo_hexdump(buf_prev, len_prev));

		/* Call item's processing handler */
		rv = pq_item->proc(pq_item->state, pq_item->buf, buf_prev, len_prev);
		assert(rv > 0);

		printf("Block '%s/%s/%s' out (len=%d): %s\n", pq->name,
			pq_item->cat_name, pq_item->sub_name, rv,
			osmo_hexdump(pq_item->buf, rv));

		buf_prev = pq_item->buf;
		len_prev = rv;
	}
}

void test_fr_concealment(struct osmo_gapk_pq *pq)
{
	uint8_t fb[GSM_FR_BYTES];
	int i;

	/* Init frame buffer with BFI */
	memset(fb, 0x00, sizeof(fb));
	fb[0] = 0xd0;

	/* Process a BFI frame */
	printf("[i] Process a BFI frame: %s\n", osmo_hexdump(fb, sizeof(fb)));
	pq_execute(pq, fb, sizeof(fb));
	printf("\n");

	/* Parse frame from string to hex */
	osmo_hexparse(sample_frame_hex, fb, GSM_FR_BYTES);

	/* Process a good frame (reset ECU) */
	printf("[i] Process a good frame: %s\n", osmo_hexdump(fb, sizeof(fb)));
	pq_execute(pq, fb, sizeof(fb));
	printf("\n");

	/* Now pretend that we do not receive any good frames anymore */
	memset(fb, 0x00, sizeof(fb));
	fb[0] = 0xd0;

	printf("[i] Pretend that we do not receive any good frames anymore\n");
	for (i = 0; i < 20; i++)
		pq_execute(pq, fb, sizeof(fb));

	printf("\n");
}

int main(int argc, char **argv)
{
	const struct osmo_gapk_codec_desc *codec;
	struct osmo_gapk_pq *pq;
	int rc;

	/* Enable tracking the use of NULL memory contexts */
	talloc_enable_null_tracking();

	/* Allocate a single processing chain */
	pq = osmo_gapk_pq_create("pq_ecu_test");
	assert(pq != NULL);

	/* Obtain FR codec description */
	codec = osmo_gapk_codec_get_from_type(CODEC_FR);
	assert(codec->ecu_proc);

	/* Put a FR ECU */
	rc = osmo_gapk_pq_queue_ecu(pq, codec);
	assert(rc == 0);

	/* Put a FR decoder */
	rc = osmo_gapk_pq_queue_codec(pq, codec, 0);
	assert(rc == 0);

	/* Prepare the chain */
	rc = osmo_gapk_pq_prepare(pq);
	assert(rc == 0);

	test_fr_concealment(pq);

	/* Release memory */
	osmo_gapk_pq_destroy(pq);

	/* Make sure we have no memleaks */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);

	return 0;
}
