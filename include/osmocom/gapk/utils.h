/* Various helpers */

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

#pragma once

#include <stdint.h>

static inline int
msb_get_bit(const uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = 7 - (bn & 7);

	return (buf[pos_byte] >> pos_bit) & 1;
}

static inline void
msb_put_bit(uint8_t *buf, int bn, int bit)
{
	int pos_byte = bn >> 3;
	int pos_bit  = 7 - (bn & 7);

	if (bit)
		buf[pos_byte] |=  (1 << pos_bit);
	else
		buf[pos_byte] &= ~(1 << pos_bit);
}

static inline void
msb_set_bit(uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = 7 - (bn & 7);

	buf[pos_byte] |=  (1 << pos_bit);
}

static inline void
msb_clr_bit(uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = 7 - (bn & 7);

	buf[pos_byte] &= ~(1 << pos_bit);
}


static inline int
lsb_get_bit(const uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = bn & 7;

	return (buf[pos_byte] >> pos_bit) & 1;
}

static inline void
lsb_put_bit(uint8_t *buf, int bn, int bit)
{
	int pos_byte = bn >> 3;
	int pos_bit  = bn & 7;

	if (bit)
		buf[pos_byte] |=  (1 << pos_bit);
	else
		buf[pos_byte] &= ~(1 << pos_bit);
}

static inline void
lsb_set_bit(uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = bn & 7;

	buf[pos_byte] |=  (1 << pos_bit);
}

static inline void
lsb_clr_bit(uint8_t *buf, int bn)
{
	int pos_byte = bn >> 3;
	int pos_bit  = bn & 7;

	buf[pos_byte] &= ~(1 << pos_bit);
}
