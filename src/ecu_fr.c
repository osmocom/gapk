/* ECU (Error Concealment Unit) for FR */

/*
 * This file is part of GAPK (GSM Audio Pocket Knife).
 *
 * (C) 2018 by Vadim Yanitskiy <axilirator@gmail.com>
 *
 * All Rights Reserved
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

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include <osmocom/gapk/codecs.h>
#include <osmocom/codec/ecu.h>

int
ecu_proc_fr(void *_state, uint8_t *out, const uint8_t *in, unsigned int in_len)
{
	struct osmo_ecu_fr_state *state = (struct osmo_ecu_fr_state *) _state;
	bool backup = false;
	bool bfi = true;
	int i, rc;

	assert(in_len == FR_CANON_LEN);

	/* Check if a frame is BFI */
	for (i = 1; i < in_len; i++) {
		if (in[i] != 0x00) {
			bfi = false;
			break;
		}
	}

	/* We have got a good frame, nothing to do */
	if (!bfi) {
		osmo_ecu_fr_reset(state, (uint8_t *) in);
		memcpy(out, in, in_len);
		return in_len;
	}

	/* Check if ECU state contains a backup frame */
	for (i = 0; i < in_len; i++) {
		if (state->frame_backup[i] != 0x00) {
			backup = true;
			break;
		}
	}

	/* There is no back-up frame */
	if (!backup) {
		/* Copy BFI 'as-is' */
		memcpy(out, in, in_len);
		return in_len;
	}

	/* Attempt to perform error concealment */
	rc = osmo_ecu_fr_conceal(state, out);
	if (rc)
		return rc;

	return in_len;
}
