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

#pragma once

#include <stdint.h>
#include <stdio.h> /* for FILE */

struct pq_item {
	/*! input frame size (in bytes). '0' in case of variable frames */
	int len_in;
	/*! output frame size (in bytes). '0' in case of variable frames */
	int len_out;
	/*! opaque state */
	void *state;
	/*! call-back for actual format conversion function
	 *  \param[in] state opaque state pointer
	 *  \param[out] out caller-allocated buffer for output data
	 *  \param[in] in input data
	 *  \param[in] in_len length of input data \a in
	 *  \returns number of output bytes written to \a out; negative on error */
	int  (*proc)(void *state, uint8_t *out, const uint8_t *in, unsigned int in_len);
	void (*exit)(void *state);
};

#define VAR_BUF_SIZE	320
#define MAX_PQ_ITEMS	8

struct pq {
	unsigned n_items;
	struct pq_item *items[MAX_PQ_ITEMS];
	void *buffers[MAX_PQ_ITEMS + 1];
};

/* Management */
struct pq *		pq_create(void);
void			pq_destroy(struct pq *pq);
struct pq_item *	pq_add_item(struct pq *pq);
int			pq_prepare(struct pq *pq);
int			pq_execute(struct pq *pq);

/* File */
int pq_queue_file_input(struct pq *pq, FILE *src, unsigned int block_len);
int pq_queue_file_output(struct pq *pq, FILE *dst, unsigned int block_len);

/* RTP */
int pq_queue_rtp_input(struct pq *pq, int rtp_fd, unsigned int block_len);
int pq_queue_rtp_output(struct pq *pq, int rtp_fd, unsigned int block_len);

/* ALSA */
int pq_queue_alsa_input(struct pq *pq, const char *hwdev, unsigned int blk_len);
int pq_queue_alsa_output(struct pq *pq, const char *hwdev, unsigned int blk_len);

/* Format */
struct format_desc;
int pq_queue_fmt_convert(struct pq *pq, const struct format_desc *fmt, int to_from_n);

/* Codec */
struct codec_desc;
int pq_queue_codec(struct pq *pq, const struct codec_desc *codec, int encode);
