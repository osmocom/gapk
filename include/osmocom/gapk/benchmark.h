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
 *
 * (C) 2014 Harald Welte <laforge@gnumonks.org>
 */

#pragma once

#include <osmocom/gapk/get_cycles.h>
#include <osmocom/gapk/codecs.h>

#define OSMO_GAPK_CYCLES_NUM_AVG 102400

struct osmo_gapk_bench_cycles {
	cycles_t enc[OSMO_GAPK_CYCLES_NUM_AVG];
	unsigned int enc_used;
	cycles_t dec[OSMO_GAPK_CYCLES_NUM_AVG];
	unsigned int dec_used;
};

extern struct osmo_gapk_bench_cycles *osmo_gapk_bench_codec[_CODEC_MAX];

int osmo_gapk_bench_enable(enum osmo_gapk_codec_type codec);
void osmo_gapk_bench_free(void);
