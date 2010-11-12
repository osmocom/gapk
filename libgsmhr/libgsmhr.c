/* HR (GSM 06.20) codec wrapper */

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

#include <stdint.h>
#include <stdio.h>

#include <gsmhr/gsmhr.h>


#define EXPORT __attribute__((visibility("default")))


struct gsmhr {
	int stub;
};

EXPORT struct gsmhr *
gsmhr_init(void)
{
	return NULL;
}

EXPORT void
gsmhr_exit(struct gsmhr *state)
{
	return;
}

EXPORT int
gsmhr_encode(struct gsmhr *state, int16_t *hr_params, const int16_t *pcm)
{
	return 0;
}

EXPORT int
gsmhr_decode(struct gsmhr *state, int16_t *pcm, const int16_t *hr_params)
{
	return 0;
}
