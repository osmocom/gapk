/* Process Queue: RTP handling tasks */

/* (C) 2013 by Harald Welte <laforge@gnumonks.org>
 *
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
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/procqueue.h>

#ifndef __BYTE_ORDER
# ifdef __APPLE__
#  define __BYTE_ORDER __DARWIN_BYTE_ORDER
#  define __LITTLE_ENDIAN __DARWIN_LITTLE_ENDIAN
#  define __BIG_ENDIAN __DARWIN_BIG_ENDIAN
# else
#  error "__BYTE_ORDER should be defined by someone"
# endif
#endif

/* according to RFC 3550 */
struct rtp_hdr {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t  csrc_count:4,
		  extension:1,
		  padding:1,
		  version:2;
	uint8_t  payload_type:7,
		  marker:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
	uint8_t  version:2,
		  padding:1,
		  extension:1,
		  csrc_count:4;
	uint8_t  marker:1,
		  payload_type:7;
#endif
	uint16_t sequence;
	uint32_t timestamp;
	uint32_t ssrc;
} __attribute__((packed));

struct rtp_x_hdr {
	uint16_t by_profile;
	uint16_t length;
} __attribute__((packed));

#define RTP_VERSION	2

#define RTP_PT_GSM_FULL 3

struct pq_state_rtp {
	int fd;
	int blk_len;

	/* configurable */
	uint32_t duration;
	uint8_t payload_type;

	/* auto-increment */
	uint16_t sequence;
	uint32_t timestamp;
	uint32_t ssrc;
};


static int
pq_cb_rtp_input(void *_state, uint8_t *out, const uint8_t *in)
{
	struct pq_state_rtp *state = _state;
	uint8_t buf[state->blk_len+256];
	struct rtp_hdr *rtph = (struct rtp_hdr *)buf;
	struct rtp_x_hdr *rtpxh;
	uint8_t *payload;
	int rv, x_len, payload_len;

	rv = read(state->fd, buf, sizeof(buf));
	if (rv <= 0) {
		perror("RTP read");
		return -1;
	}

	if (rv < sizeof(struct rtp_hdr)) {
		fprintf(stderr, "%d smaller than rtp header\n", rv);
		return -1;
	}

	if (rtph->version != RTP_VERSION) {
		fprintf(stderr, "unknown RTP version %u\n", rtph->version);
		return -1;
	}

	payload = buf + sizeof(struct rtp_hdr) + (rtph->csrc_count << 2);
	payload_len = rv - sizeof(struct rtp_hdr) - (rtph->csrc_count << 2);
	if (payload_len < 0) {
		fprintf(stderr, "non-existant RTP payload length %d\n", payload_len);
		return -1;
	}

	if (rtph->extension) {
		if (payload_len < sizeof(struct rtp_x_hdr)) {
			fprintf(stderr, "short extension header: %d\n", payload_len);
			return -1;
		}
		rtpxh = (struct rtp_x_hdr *)payload;
		x_len = ntohs(rtpxh->length) * 4 + sizeof(struct rtp_x_hdr);
		payload += x_len;
		payload_len -= x_len;
		if (payload_len < 0) {
			fprintf(stderr, "short RTP payload length %d\n", payload_len);
			return -1;
		}
	}
	if (rtph->padding) {
		if (payload_len < 0) {
			fprintf(stderr, "padding but no payload length %d\n", payload_len);
			return -1;
		}
		payload_len -= payload[payload_len -1];
		if (payload_len < 0) {
			fprintf(stderr, "no payload left after padding %d\n", payload_len);
			return -1;
		}
	}

	state->ssrc = ntohl(rtph->ssrc);
	state->timestamp = ntohl(rtph->timestamp);
	state->sequence = ntohs(rtph->sequence);
	/* FIXME: check for discontinuity, ... */

	memcpy(out, payload, payload_len);

	return 0;
}

static int
pq_cb_rtp_output(void *_state, uint8_t *out, const uint8_t *in)
{
	struct pq_state_rtp *state = _state;
	int len = state->blk_len + sizeof(struct rtp_hdr);
	uint8_t buf[len];
	struct rtp_hdr *rtph = (struct rtp_hdr *)buf;
	uint8_t *payload;
	int rv;

	rtph->version = RTP_VERSION;
	rtph->padding = 0;
	rtph->extension = 0;
	rtph->csrc_count = 0;
	rtph->marker = 0;
	rtph->payload_type = state->payload_type;
	rtph->sequence = htons(state->sequence++);
	rtph->timestamp = htonl(state->timestamp);
	state->timestamp += state->duration;
	rtph->ssrc = htonl(state->ssrc);

	payload = buf + sizeof(*rtph);
	memcpy(payload, in, state->blk_len);

	rv = write(state->fd, buf, len);
	return rv == len ? 0 : -1;
}

static void
pq_cb_rtp_exit(void *_state)
{
	free(_state);
}

static int
pq_queue_rtp_op(struct pq *pq, int udp_fd, unsigned int blk_len, int in_out_n)
{
	struct pq_item *item;
	struct pq_state_rtp *state;

	state = calloc(1, sizeof(struct pq_state_rtp));
	if (!state)
		return -ENOMEM;

	state->fd = udp_fd;
	state->blk_len = blk_len;

	/* as we're working in GSM, the sample clock is 8000 Hz and we
	 * operate at 50 Hz (20ms) codec frames; 8000/50 = 160 samples
	 * per RTP frame */
	state->duration = 160;

	if (in_out_n == 0) {
		state->ssrc = rand();
		state->sequence = random();
		state->timestamp = random();
		/* FIXME: other payload types!! */
		state->payload_type = RTP_PT_GSM_FULL;
	}

	item = pq_add_item(pq);
	if (!item) {
		free(state);
		return -ENOMEM;
	}

	item->len_in  = in_out_n ? 0 : blk_len;
	item->len_out = in_out_n ? blk_len : 0;
	item->state   = state;
	item->proc    = in_out_n ? pq_cb_rtp_input : pq_cb_rtp_output;
	item->exit    = pq_cb_rtp_exit;

	return 0;
}


/*! Add RTP input to processing queue.
 *  This typically only makes sense as first item in the queue
 *  \param pq Processing Queue to add this RTP input to
 *  \param[in] udp_fd UDP file descriptor for the RTP input
 *  \param[in] blk_len Block Length to read from RTP */
int
pq_queue_rtp_input(struct pq *pq, int udp_fd, unsigned int blk_len)
{
	return pq_queue_rtp_op(pq, udp_fd, blk_len, 1);
}

/*! Add RTP output to processing queue.
 *  This typically only makes sense as last item in the queue
 *  \param pq Processing Queue to add this RTP output to
 *  \param[in] udp_fd UDP file descriptor for the RTP output
 *  \param[in] blk_len Block Length to read from RTP */
int
pq_queue_rtp_output(struct pq *pq, int udp_fd, unsigned int blk_len)
{
	return pq_queue_rtp_op(pq, udp_fd, blk_len, 0);
}
