/* Processing Queue Management */

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

#ifndef __GAPK_PROCQUEUE_H__
#define __GAPK_PROCQUEUE_H__

#include <stdint.h>
#include <stdio.h> /* for FILE */

struct pq;

struct pq_item {
	int len_in;
	int len_out;
	void *state;
	int  (*proc)(void *state, uint8_t *out, const uint8_t *in);
	void (*exit)(void *state);
};

/* Management */
struct pq *		pq_create(void);
void			pq_destroy(struct pq *pq);
struct pq_item *	pq_add_item(struct pq *pq);
int			pq_prepare(struct pq *pq);
int			pq_execute(struct pq *pq);

#endif /* __GAPK_PROCQUEUE_H__ */
