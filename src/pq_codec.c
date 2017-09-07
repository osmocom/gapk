/* Process Queue: Codec handling tasks */

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

#include <osmocom/gapk/logging.h>
#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/procqueue.h>


/*! Add a codecl to the processing queue
 *  \param pq Processing Queue to which the codec is added
 *  \param[in] codec description
 *  \param[in] encode (1) or decode (0)
 *  \returns 0 on success; negative on error */
int
osmo_gapk_pq_queue_codec(struct osmo_gapk_pq *pq, const struct osmo_gapk_codec_desc *codec, int enc_dec_n)
{
	const struct osmo_gapk_codec_desc *codec_pcm;
	const struct osmo_gapk_format_desc *fmt;
	struct osmo_gapk_pq_item *item;

	/* allocate a new item to the processing queue */
	item = osmo_gapk_pq_add_item(pq);
	if (!item)
		return -ENOMEM;

	/* initialize the codec, if there is an init function */
	if (codec->codec_init) {
		item->state = codec->codec_init();
		if (!item->state)
			return -EINVAL;
	}

	if (enc_dec_n) {
		codec_pcm = osmo_gapk_codec_get_from_type(CODEC_PCM);
		fmt = osmo_gapk_fmt_get_from_type(codec->codec_enc_format_type);
		if (!fmt)
			return -EINVAL;

		item->len_in  = codec_pcm->canon_frame_len;
		item->len_out = fmt->frame_len;
		item->proc    = codec->codec_encode;
	} else {
		codec_pcm = osmo_gapk_codec_get_from_type(CODEC_PCM);
		fmt = osmo_gapk_fmt_get_from_type(codec->codec_dec_format_type);
		if (!fmt)
			return -EINVAL;

		item->len_in  = fmt->frame_len;
		item->len_out = codec_pcm->canon_frame_len;
		item->proc    = codec->codec_decode;
	}

	item->exit = codec->codec_exit;
	item->wait = NULL;

	LOGPGAPK(LOGL_DEBUG, "PQ: Adding codec %s, %s format %s\n", codec->name,
		enc_dec_n ? "encoding to" : "decoding from", fmt->name);

	if (!item->proc)
		return -ENOTSUP;

	return 0;
}
