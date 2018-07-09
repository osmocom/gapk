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

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <talloc.h>
#include <assert.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osmocom/core/socket.h>

#include <osmocom/gapk/procqueue.h>
#include <osmocom/gapk/common.h>

/**
 * This test is intended to check the RTP source / sink operability.
 * To do this, two processing queues are being allocated:
 *
 *     "generator": source/random -> sink/rtp
 *     "checker": source/rtp -> sink/checker
 *
 * The first one generates some amount of random bytes (payload),
 * and stores them inside a buffer that is shared between both
 * queues.
 *
 * After generation, a payload is being sent from the first
 * queue via an RTP sink, and then being received by the second
 * via an RTP source.
 *
 * As both queues do use a shared buffer, the last item of the
 * second queue (named 'sink/checker') is able to compare a
 * received payload with expected.
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

#define RTP_TEST_BUF_LEN 128

struct rtp_test_state {
	unsigned int payload_len;
	unsigned int rtp_port;
	int rtp_src_fd;
	int rtp_dst_fd;

	uint8_t data[RTP_TEST_BUF_LEN];
	uint8_t *ptr;
};

static int src_rand_proc(void *data, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	struct rtp_test_state *state = (struct rtp_test_state *) data;
	unsigned int i;

	/* Generate a random payload */
	for (i = 0; i < state->payload_len; i++) {
		uint8_t byte = rand() % 0xff;
		*(state->ptr + i) = byte;
		out[i] = byte;
	}

	return state->payload_len;
}

static void src_rand_exit(void *data)
{
	struct rtp_test_state *state = (struct rtp_test_state *) data;

	if (state->rtp_src_fd >= 0) {
		close(state->rtp_src_fd);
		state->rtp_src_fd = -1;
	}
}

static int sink_chk_proc(void *data, uint8_t *out, const uint8_t *in,
	unsigned int in_len)
{
	struct rtp_test_state *state = (struct rtp_test_state *) data;
	unsigned int i;

	/* Make sure we have all bytes transferred */
	if (in_len != state->payload_len) {
		printf("Data length mismatch!\n");
		return -EINVAL;
	}

	for (i = 0; i < in_len; i++) {
		if (in[i] != *(state->ptr + i)) {
			printf("Data mismatch!\n");
			return -EINVAL;
		}
	}

	return in_len;
}

static void sink_chk_exit(void *data)
{
	struct rtp_test_state *state = (struct rtp_test_state *) data;

	if (state->rtp_dst_fd >= 0) {
		close(state->rtp_dst_fd);
		state->rtp_dst_fd = -1;
	}
}

/* Allocates: source/random -> sink/rtp */
static int init_gen_queue(struct osmo_gapk_pq *pq,
	struct rtp_test_state *state, unsigned int payload_len)
{
	int rc;

	/* Allocate memory for the 'source/random' */
	struct osmo_gapk_pq_item *src_rand = osmo_gapk_pq_add_item(pq);
	if (!src_rand)
		return -ENOMEM;

	/* Fill in meta information */
	src_rand->type = OSMO_GAPK_ITEM_TYPE_SOURCE;
	src_rand->cat_name = OSMO_GAPK_CAT_NAME_SOURCE;
	src_rand->sub_name = "random";

	/* Set I/O buffer lengths */
	state->payload_len = payload_len;
	src_rand->len_out = payload_len;

	/* Set proc / exit callbacks and state */
	src_rand->proc = &src_rand_proc;
	src_rand->exit = &src_rand_exit;
	src_rand->state = state;


	/* Init connection socket */
	state->rtp_dst_fd = osmo_sock_init(AF_UNSPEC, SOCK_DGRAM,
		IPPROTO_UDP, "127.0.0.1", state->rtp_port, OSMO_SOCK_F_CONNECT);
	if (state->rtp_dst_fd < 0) {
		printf("Could not init connection socket\n");
		return -EINVAL;
	}

	/* Init an RTP sink */
	rc = osmo_gapk_pq_queue_rtp_output(pq, state->rtp_dst_fd, payload_len, 0x00);
	if (rc) {
		printf("Could not init an RTP sink\n");
		return rc;
	}

	/* Check and prepare */
	rc = osmo_gapk_pq_check(pq, 1);
	if (rc) {
		printf("Queue check failed\n");
		return rc;
	}

	rc = osmo_gapk_pq_prepare(pq);
	if (rc) {
		printf("Queue preparation failed\n");
		return rc;
	}

	return 0;
}

/* Allocates: source/rtp -> sink/checker */
static int init_chk_queue(struct osmo_gapk_pq *pq,
	struct rtp_test_state *state, unsigned int payload_len)
{
	int rc;

	/* Init listening socket */
	state->rtp_src_fd = osmo_sock_init(AF_UNSPEC, SOCK_DGRAM,
		IPPROTO_UDP, "127.0.0.1", 0, OSMO_SOCK_F_BIND);
	if (state->rtp_src_fd < 0) {
		printf("Could not init listening socket\n");
		return -EINVAL;
	}

	/* Init an RTP source on any available port */
	rc = osmo_gapk_pq_queue_rtp_input(pq, state->rtp_src_fd, payload_len, 0x00);
	if (rc) {
		printf("Could not init an RTP sink\n");
		return rc;
	}

	/* Determine on which port are we listening */
	struct sockaddr_in adr_inet;
	socklen_t len_inet;

	len_inet = sizeof(adr_inet);
	rc = getsockname(state->rtp_src_fd,
		(struct sockaddr *) &adr_inet, &len_inet);
	if (rc)
		return -EINVAL;

	/* Save assigned port to shared state */
	state->rtp_port = (unsigned int) ntohs(adr_inet.sin_port);


	/* Allocate memory for the 'sink/checker' */
	struct osmo_gapk_pq_item *sink_chk = osmo_gapk_pq_add_item(pq);
	if (!sink_chk)
		return -ENOMEM;

	/* Fill in meta information */
	sink_chk->type = OSMO_GAPK_ITEM_TYPE_SINK;
	sink_chk->cat_name = OSMO_GAPK_CAT_NAME_SINK;
	sink_chk->sub_name = "checker";

	/* Set I/O buffer lengths */
	sink_chk->len_in = payload_len;

	/* Set proc / exit callbacks and state */
	sink_chk->proc = &sink_chk_proc;
	sink_chk->exit = &sink_chk_exit;
	sink_chk->state = state;

	/* Check and prepare */
	rc = osmo_gapk_pq_check(pq, 1);
	if (rc) {
		printf("Queue check failed\n");
		return rc;
	}

	rc = osmo_gapk_pq_prepare(pq);
	if (rc) {
		printf("Queue preparation failed\n");
		return rc;
	}

	return 0;
}

static int rtp_test(struct rtp_test_state *state, unsigned int payload_len)
{
	struct osmo_gapk_pq *q_gen, *q_chk;
	unsigned int i, chunks;
	int rc;

	/* Allocate two queues */
	q_gen = osmo_gapk_pq_create("generator");
	q_chk = osmo_gapk_pq_create("checker");

	/* Make sure both queues are allocated */
	if (!q_gen || !q_chk) {
		rc = -ENOMEM;
		goto exit;
	}

	/* Init both queues: generator and checker */
	rc = init_chk_queue(q_chk, state, payload_len);
	if (rc)
		goto exit;

	rc = init_gen_queue(q_gen, state, payload_len);
	if (rc)
		goto exit;

	/* Calculate how much chunks do we have */
	chunks = RTP_TEST_BUF_LEN / payload_len;

	/* Execute both queues */
	for (i = 0; i < chunks; i++) {
		/* Move data pointer */
		state->ptr = state->data + i * payload_len;

		/* Generate and send a payload */
		rc = osmo_gapk_pq_execute(q_gen);
		if (rc) {
			printf("Queue '%s' execution aborted on chunk %u/%u\n",
				q_gen->name, i + 1, chunks);
			goto exit;
		}

		/* TODO: prevent test hang if nothing was being sent */

		/* Receive and check a payload */
		rc = osmo_gapk_pq_execute(q_chk);
		if (rc) {
			printf("Queue '%s' execution aborted on chunk %u/%u\n",
				q_gen->name, i + 1, chunks);
			goto exit;
		}
	}

	printf("Payload len=%u check ok\n", payload_len);

exit:
	/* Deallocate both queues and data */
	osmo_gapk_pq_destroy(q_gen);
	osmo_gapk_pq_destroy(q_chk);

	return rc;
}

int main(int argc, char **argv)
{
	struct rtp_test_state state;
	unsigned int len;

	/* Enable tracking the use of NULL memory contexts */
	talloc_enable_null_tracking();

	/* Init pseudo-random generator */
	srand(time(NULL));

	/* Perform testing with different payload size values */
	for (len = 1; len <= RTP_TEST_BUF_LEN; len *= 2)
		assert(rtp_test(&state, len) == 0);
	printf("\n");

	/* Memory leak detection test */
	talloc_report_depth_cb(NULL, 0, 10, &talloc_ctx_walk_cb, NULL);

	/* Make both Valgrind and LeakSanitizer happy */
	talloc_disable_null_tracking();

	return 0;
}
