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

#include <gapk/codecs.h>
#include <gapk/formats.h>
#include <gapk/procqueue.h>


/*! Add a codecl to the processing queue
 *  \param pq Processing Queue to which the codec is added
 *  \param[in] codec description
 *  \param[in] encode (1) or decode (0)
 *  \returns 0 on success; negative on error */
int
pq_queue_codec(struct pq *pq, const struct codec_desc *codec, int enc_dec_n)
{
	const struct codec_desc *codec_pcm = codec_get_from_type(CODEC_PCM);
	const struct format_desc *fmt;
	struct pq_item *item;

	/* allocate a new item to the processing queue */
	item = pq_add_item(pq);
	if (!item)
		return -ENOMEM;

	/* initialize the codec, if there is an init function */
	if (codec->codec_init) {
		item->state = codec->codec_init();
		if (!item->state)
			return -EINVAL;
	}

	if (enc_dec_n) {
		fmt = fmt_get_from_type(codec->codec_enc_format_type);
		if (!fmt)
			return -EINVAL;

		item->len_in  = codec_pcm->canon_frame_len;
		item->len_out = fmt->frame_len;
		item->proc    = codec->codec_encode;
	} else {
		fmt = fmt_get_from_type(codec->codec_dec_format_type);
		if (!fmt)
			return -EINVAL;

		item->len_in  = fmt->frame_len;
		item->len_out = codec_pcm->canon_frame_len;
		item->proc    = codec->codec_decode;
	}

	item->exit = codec->codec_exit;

	if (!item->proc)
		return -ENOTSUP;

	return 0;
}
