/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * (C) 2014 Harald Welte <laforge@gnumonks.org>
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
 */

#pragma once

#include <osmocom/gapk/get_cycles.h>
#include <osmocom/gapk/benchmark.h>
#include <osmocom/gapk/codecs.h>

static inline void benchmark_store(enum osmo_gapk_codec_type codec,
	int encode, unsigned long cycles)
{
	struct osmo_gapk_bench_cycles *bc = &osmo_gapk_bench_codec[codec];

	if (encode) {
		bc->enc_used = (bc->enc_used + 1) % OSMO_GAPK_CYCLES_NUM_AVG;
		bc->enc[bc->enc_used] = cycles;
	} else {
		bc->dec_used = (bc->dec_used + 1) % OSMO_GAPK_CYCLES_NUM_AVG;
		bc->dec[bc->dec_used] = cycles;
	}
}	

#define BENCHMARK_START					\
	do {						\
		cycles_t _cycles_start, _cycles_stop;	\
		_cycles_start = get_cycles()

#define BENCHMARK_STOP(codec, enc)			\
		_cycles_stop = get_cycles();		\
		benchmark_store(codec, enc,		\
			_cycles_stop - _cycles_start);	\
	} while (0)
