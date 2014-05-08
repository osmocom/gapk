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

#include <stdio.h>

#include <gapk/benchmark.h>

struct benchmark_cycles codec_cycles[_CODEC_MAX];

void benchmark_dump(void)
{
	int i;

	for (i = 0; i < _CODEC_MAX; i++) {
		struct benchmark_cycles *bc = &codec_cycles[i];
		unsigned long long total;
		int j;

		if (bc->enc_used) {
			total = 0;
			for (j = 0; j < bc->enc_used; j++)
				total += bc->enc[j];

			printf("Codec %u (ENC): %llu cycles for %u frames => "
				"%llu cycles/frame\n", i, total, bc->enc_used,
				total / bc->enc_used);
		}

		if (bc->dec_used) {
			total = 0;
			for (j = 0; j < bc->dec_used; j++)
				total += bc->dec[j];

			printf("Codec %u (DEC): %llu cycles for %u frames => "
				"%llu cycles/frame\n", i, total, bc->dec_used,
				total / bc->dec_used);
		}
	}
}
