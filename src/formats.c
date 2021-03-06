/* Formats handling */

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
#include <string.h>

#include <osmocom/gapk/formats.h>

/* Extern format descriptors */
extern const struct osmo_gapk_format_desc fmt_amr_efr;
extern const struct osmo_gapk_format_desc fmt_gsm;
extern const struct osmo_gapk_format_desc fmt_hr_ref_dec;
extern const struct osmo_gapk_format_desc fmt_hr_ref_enc;
extern const struct osmo_gapk_format_desc fmt_racal_hr;
extern const struct osmo_gapk_format_desc fmt_racal_fr;
extern const struct osmo_gapk_format_desc fmt_racal_efr;
extern const struct osmo_gapk_format_desc fmt_rawpcm_s16le;
extern const struct osmo_gapk_format_desc fmt_ti_hr;
extern const struct osmo_gapk_format_desc fmt_ti_fr;
extern const struct osmo_gapk_format_desc fmt_ti_efr;
extern const struct osmo_gapk_format_desc fmt_amr_opencore;
extern const struct osmo_gapk_format_desc fmt_rtp_amr;
extern const struct osmo_gapk_format_desc fmt_rtp_efr;
extern const struct osmo_gapk_format_desc fmt_rtp_hr_etsi;
extern const struct osmo_gapk_format_desc fmt_rtp_hr_ietf;

static const struct osmo_gapk_format_desc *supported_formats[_FMT_MAX] = {
	[FMT_INVALID]		= NULL,
	[FMT_AMR_EFR]		= &fmt_amr_efr,
	[FMT_GSM]		= &fmt_gsm,
	[FMT_HR_REF_DEC]	= &fmt_hr_ref_dec,
	[FMT_HR_REF_ENC]	= &fmt_hr_ref_enc,
	[FMT_RACAL_HR]		= &fmt_racal_hr,
	[FMT_RACAL_FR]		= &fmt_racal_fr,
	[FMT_RACAL_EFR]		= &fmt_racal_efr,
	[FMT_RAWPCM_S16LE]	= &fmt_rawpcm_s16le,
	[FMT_TI_HR]		= &fmt_ti_hr,
	[FMT_TI_FR]		= &fmt_ti_fr,
	[FMT_TI_EFR]		= &fmt_ti_efr,
	[FMT_AMR_OPENCORE]	= &fmt_amr_opencore,
	[FMT_RTP_AMR]		= &fmt_rtp_amr,
	[FMT_RTP_EFR]		= &fmt_rtp_efr,
	[FMT_RTP_HR_ETSI]	= &fmt_rtp_hr_etsi,
	[FMT_RTP_HR_IETF]	= &fmt_rtp_hr_ietf,
};


const struct osmo_gapk_format_desc *
osmo_gapk_fmt_get_from_type(enum osmo_gapk_format_type type)
{
	if (type <= FMT_INVALID || type >= _FMT_MAX)
		return NULL;
	return supported_formats[type];
}

const struct osmo_gapk_format_desc *
osmo_gapk_fmt_get_from_name(const char *name)
{
	int i;
	for (i=FMT_INVALID+1; i<_FMT_MAX; i++) {
		const struct osmo_gapk_format_desc *fmt = supported_formats[i];
		if (!fmt)
			continue;
		if (!strcmp(fmt->name, name))
			return fmt;
	}
	return NULL;
}
