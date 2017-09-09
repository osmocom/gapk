/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * (C) 2017 by Vadim Yanitskiy <axilirator@gmail.com>
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

/* Internal GAPK logging */
int osmo_gapk_log_init_complete = 0;
int osmo_gapk_log_subsys;

void osmo_gapk_log_init(int subsys)
{
	osmo_gapk_log_subsys = subsys;
	osmo_gapk_log_init_complete = 1;
}
