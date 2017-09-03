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

#define BENCHMARK_START					\
	do {						\
		cycles_t _cycles_start, _cycles_stop;	\
		struct osmo_gapk_bench_cycles *_bc;	\
		_cycles_start = get_cycles()

#define BENCHMARK_STOP(codec, encode)				\
		_cycles_stop = get_cycles();			\
		_bc = &osmo_gapk_bench_codec[codec];		\
								\
		if (encode) {					\
			_bc->enc_used = (_bc->enc_used + 1)	\
				% OSMO_GAPK_CYCLES_NUM_AVG;	\
			_bc->enc[_bc->enc_used] =		\
				_cycles_stop - _cycles_start;	\
		} else {					\
			_bc->dec_used = (_bc->dec_used + 1)	\
				% OSMO_GAPK_CYCLES_NUM_AVG;	\
			_bc->dec[_bc->dec_used] =		\
				_cycles_stop - _cycles_start;	\
		}						\
	} while (0)
