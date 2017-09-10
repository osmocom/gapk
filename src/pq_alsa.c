/* Process Queue: ALSA PCM output */
/* (C) 2017 by Harald Welte <laforge@gnumonks.org> */

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
#include <talloc.h>

#include <osmocom/gapk/logging.h>
#include <osmocom/gapk/codecs.h>
#include <osmocom/gapk/formats.h>
#include <osmocom/gapk/procqueue.h>

#include "config.h"

#ifdef HAVE_ALSA

#include <alsa/asoundlib.h>


struct pq_state_alsa {
	snd_pcm_t *pcm_handle;
	unsigned int blk_len;
};

static int
pq_cb_alsa_input(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	struct pq_state_alsa *state = _state;
	unsigned int num_samples = state->blk_len/2;
	int rv;
	rv = snd_pcm_readi(state->pcm_handle, out, num_samples);
	if (rv <= 0)
		return -1;
	return rv * sizeof(uint16_t);
}

static int
pq_cb_alsa_output(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	struct pq_state_alsa *state = _state;
	unsigned int num_samples = in_len/2;
	int rv;

	rv = snd_pcm_writei(state->pcm_handle, in, num_samples);
	if (rv == -EPIPE) {
		/* Recover from buffer underrun */
		snd_pcm_prepare(state->pcm_handle);
		/* Send a new sample again */
		rv = snd_pcm_writei(state->pcm_handle, in, num_samples);
	}

	return rv == num_samples ? 0 : -1;
}

static int
pq_cb_alsa_wait(void *_state)
{
	struct pq_state_alsa *state = _state;
	return snd_pcm_avail_update(state->pcm_handle) > 0;
}

static void
pq_cb_alsa_exit(void *_state)
{
	struct pq_state_alsa *state = _state;
	snd_pcm_close(state->pcm_handle);
}

static int
pq_queue_alsa_op(struct osmo_gapk_pq *pq, const char *alsa_dev, unsigned int blk_len, int in_out_n)
{
	struct osmo_gapk_pq_item *item;
	struct pq_state_alsa *state;
	snd_pcm_hw_params_t *hw_params;
	int rc = -1;

	state = talloc_zero(pq, struct pq_state_alsa);
	if (!state) {
		rc = -ENOMEM;
		goto out_print;
	}

	rc = snd_pcm_open(&state->pcm_handle, alsa_dev,
			  in_out_n ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0)
		goto out_print;

	state->blk_len = blk_len;

	rc = snd_pcm_hw_params_malloc(&hw_params);
	if (rc < 0)
		goto out_close;

	rc = snd_pcm_hw_params_any(state->pcm_handle, hw_params);
	if (rc < 0)
		goto out_free_par;

	rc = snd_pcm_hw_params_set_access(state->pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
		goto out_free_par;

	rc = snd_pcm_hw_params_set_format(state->pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
		goto out_free_par;

	rc = snd_pcm_hw_params_set_rate(state->pcm_handle, hw_params, 8000, 0);
	if (rc < 0)
		goto out_free_par;

	rc = snd_pcm_hw_params_set_channels (state->pcm_handle, hw_params, 1);
	if (rc < 0)
		goto out_free_par;

	rc = snd_pcm_hw_params(state->pcm_handle, hw_params);
	if (rc < 0)
		goto out_free_par;

	snd_pcm_hw_params_free(hw_params);

	item = osmo_gapk_pq_add_item(pq);
	if (!item) {
		rc = -ENOMEM;
		goto out_close;
	}

	item->type = in_out_n ?
		OSMO_GAPK_ITEM_TYPE_SOURCE : OSMO_GAPK_ITEM_TYPE_SINK;
	item->cat_name = in_out_n ? "source" : "sink";
	item->sub_name = "alsa";

	item->len_in  = in_out_n ? 0 : blk_len;
	item->len_out = in_out_n ? blk_len : 0;
	item->state   = state;
	item->proc    = in_out_n ? pq_cb_alsa_input : pq_cb_alsa_output;
	item->wait    = pq_cb_alsa_wait;
	item->exit    = pq_cb_alsa_exit;

	/* Change state's talloc context from pq to item */
	talloc_steal(item, state);

	return 0;

out_free_par:
	snd_pcm_hw_params_free(hw_params);
out_close:
	snd_pcm_close(state->pcm_handle);
	talloc_free(state);
out_print:
	LOGPGAPK(LOGL_ERROR, "Couldn't init ALSA device '%s': %s\n",
		alsa_dev, snd_strerror(rc));
	return rc;
}


/*! Add ALSA input to given processing queue
 *  This usually only makes sense as first item in the queue
 *  \param pq Processing Queue to add the ALSA input to
 *  \param[in] hwdev ALSA hardware device to use
 *  \param[in] blk_len block length to be read from device
 *  \returns 0 on sucess; negative on error */
int
osmo_gapk_pq_queue_alsa_input(struct osmo_gapk_pq *pq, const char *hwdev, unsigned int blk_len)
{
	LOGPGAPK(LOGL_DEBUG, "PQ '%s': Adding ALSA input "
		"(dev='%s', blk_len=%u)\n", pq->name, hwdev, blk_len);
	return pq_queue_alsa_op(pq, hwdev, blk_len, 1);
}

/*! Add ALSA output to given processing queue
 *  This usually only makes sense as first item in the queue
 *  \param pq Processing Queue to add the ALSA output to
 *  \param[in] hwdev ALSA hardware device to use
 *  \param[in] blk_len block length to be written to device
 *  \returns 0 on sucess; negative on error */
int
osmo_gapk_pq_queue_alsa_output(struct osmo_gapk_pq *pq, const char *hwdev, unsigned int blk_len)
{
	LOGPGAPK(LOGL_DEBUG, "PQ '%s': Adding ALSA output "
		"(dev='%s', blk_len=%u)\n", pq->name, hwdev, blk_len);
	return pq_queue_alsa_op(pq, hwdev, blk_len, 0);
}

#endif /* HAVE_ALSA */
