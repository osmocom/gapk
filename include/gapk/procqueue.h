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

#ifndef __GAPK_PROCQUEUE_H__
#define __GAPK_PROCQUEUE_H__

#include <stdint.h>
#include <stdio.h> /* for FILE */

struct pq;

struct pq_item {
	int len_in;
	int len_out;
	void *state;
	int  (*proc)(void *state, uint8_t *out, const uint8_t *in, unsigned int in_len);
	void (*exit)(void *state);
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

#endif /* __GAPK_PROCQUEUE_H__ */
