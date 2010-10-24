/* Codecs handling */

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

#include <stdio.h>	/* for NULL */

#include <gapk/codecs.h>

/* Extern codec descriptors */
extern const struct codec_desc codec_pcm_desc;
extern const struct codec_desc codec_hr_desc;
extern const struct codec_desc codec_fr_desc;
extern const struct codec_desc codec_efr_desc;


const struct codec_desc *
codec_get_from_type(enum codec_type type)
{
	switch (type) {
	case CODEC_PCM:	return &codec_pcm_desc;
	case CODEC_HR:	return &codec_hr_desc;
	case CODEC_FR:	return &codec_fr_desc;
	case CODEC_EFR:	return &codec_efr_desc;
	default:
		return NULL;
	}
}
