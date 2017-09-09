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

#include <talloc.h>
#include <errno.h>

#include <osmocom/gapk/benchmark.h>
#include <osmocom/gapk/codecs.h>

struct osmo_gapk_bench_cycles *
	osmo_gapk_bench_codec[_CODEC_MAX] = { NULL };

int osmo_gapk_bench_enable(enum osmo_gapk_codec_type codec)
{
	struct osmo_gapk_bench_cycles *bench;

	/* Allocate zero-initialized memory */
	bench = talloc_zero(NULL, struct osmo_gapk_bench_cycles);
	if (!bench)
		return -ENOMEM;

	/* Set up pointer */
	osmo_gapk_bench_codec[codec] = bench;

	return 0;
}

unsigned long long
osmo_gapk_bench_get_cycles(enum osmo_gapk_codec_type codec, int enc)
{
	struct osmo_gapk_bench_cycles *bench;
	unsigned long long cycles = 0;
	int i;

	/* Check if there are benchmark data */
	bench = osmo_gapk_bench_codec[codec];
	if (!bench)
		return -EAGAIN;

	if (enc) {
		for (i = 0; i < bench->enc_used; i++)
			cycles += bench->enc[i];
	} else {
		for (i = 0; i < bench->dec_used; i++)
			cycles += bench->dec[i];
	}

	return cycles;
}

unsigned int
osmo_gapk_bench_get_frames(enum osmo_gapk_codec_type codec, int enc)
{
	struct osmo_gapk_bench_cycles *bench;

	/* Check if there are benchmark data */
	bench = osmo_gapk_bench_codec[codec];
	if (!bench)
		return -EAGAIN;

	return enc ? bench->enc_used : bench->dec_used;
}

void osmo_gapk_bench_free(void)
{
	int i;

	for (i = 0; i < _CODEC_MAX; i++)
		talloc_free(osmo_gapk_bench_codec[i]);
}
