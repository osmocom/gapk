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
 */

#pragma once

#include <osmocom/core/logging.h>

extern int gapk_log_init_complete;
extern int gapk_log_subsys;

#define LOGPGAPK(level, fmt, args...) \
	if (gapk_log_init_complete) \
		LOGP(gapk_log_subsys, level, fmt, ## args)
