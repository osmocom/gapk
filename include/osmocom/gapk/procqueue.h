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

#include <osmocom/core/linuxlist.h>

enum osmo_gapk_pq_item_type {
	OSMO_GAPK_ITEM_TYPE_SOURCE,
	OSMO_GAPK_ITEM_TYPE_SINK,
	OSMO_GAPK_ITEM_TYPE_PROC,
};

#define OSMO_GAPK_CAT_NAME_SOURCE	"source"
#define OSMO_GAPK_CAT_NAME_SINK	"sink"
#define OSMO_GAPK_CAT_NAME_PROC	"proc"

struct osmo_gapk_pq_item {
	/*! input frame size (in bytes). '0' in case of variable frames */
	unsigned int len_in;
	/*! output frame size (in bytes). '0' in case of variable frames */
	unsigned int len_out;
	/*! opaque state */
	void *state;
	/*! buffer for output data */
	uint8_t *buf;
	/*! call-back for actual format conversion function
	 *  \param[in] state opaque state pointer
	 *  \param[out] out caller-allocated buffer for output data
	 *  \param[in] in input data
	 *  \param[in] in_len length of input data \a in
	 *  \returns number of output bytes written to \a out; negative on error */
	int  (*proc)(void *state, uint8_t *out, const uint8_t *in, unsigned int in_len);
	int  (*wait)(void *state);
	void (*exit)(void *state);

	/*! \brief link to a processing queue */
	struct llist_head list;
	/*! \brief type of item */
	enum osmo_gapk_pq_item_type type;
	/*! \brief category name (src, format, codec, sink) */
	const char *cat_name;
	/*! \brief sub-category name (file, rtp-amr, amr, alsa) */
	const char *sub_name;
};

#define VAR_BUF_SIZE	320

struct osmo_gapk_pq {
	struct llist_head items;
	unsigned n_items;

	/*! \brief human-readable name */
	const char *name;
};

/* Processing queue management */
struct osmo_gapk_pq *osmo_gapk_pq_create(const char *name);
int osmo_gapk_pq_check(struct osmo_gapk_pq *pq, int strict);
int osmo_gapk_pq_prepare(struct osmo_gapk_pq *pq);
int osmo_gapk_pq_execute(struct osmo_gapk_pq *pq);
void osmo_gapk_pq_destroy(struct osmo_gapk_pq *pq);
char *osmo_gapk_pq_describe(struct osmo_gapk_pq *pq);

/* Processing queue item management */
struct osmo_gapk_pq_item *osmo_gapk_pq_add_item(struct osmo_gapk_pq *pq);

/* File */
int osmo_gapk_pq_queue_file_input(struct osmo_gapk_pq *pq, FILE *src, unsigned int block_len);
int osmo_gapk_pq_queue_file_output(struct osmo_gapk_pq *pq, FILE *dst, unsigned int block_len);

/* RTP */
int osmo_gapk_pq_queue_rtp_input(struct osmo_gapk_pq *pq, int rtp_fd, unsigned int block_len);
int osmo_gapk_pq_queue_rtp_output(struct osmo_gapk_pq *pq, int rtp_fd, unsigned int block_len);

/* ALSA */
int osmo_gapk_pq_queue_alsa_input(struct osmo_gapk_pq *pq, const char *hwdev, unsigned int blk_len);
int osmo_gapk_pq_queue_alsa_output(struct osmo_gapk_pq *pq, const char *hwdev, unsigned int blk_len);

/* Format */
struct osmo_gapk_format_desc;
int osmo_gapk_pq_queue_fmt_convert(struct osmo_gapk_pq *pq, const struct osmo_gapk_format_desc *fmt, int to_from_n);

/* Codec */
struct osmo_gapk_codec_desc;
int osmo_gapk_pq_queue_codec(struct osmo_gapk_pq *pq, const struct osmo_gapk_codec_desc *codec, int encode);
