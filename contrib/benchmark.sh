#!/bin/sh

GAPK=./src/gapk
PCMFILE=$1
BASE=`basename $PCMFILE`

echo
echo Encoder Benchmark:
$GAPK -i "$PCMFILE" -f rawpcm-s16le -o "$BASE.fr" -g gsm
$GAPK -i "$PCMFILE" -f rawpcm-s16le -o "$BASE.efr" -g amr-efr
$GAPK -i "$PCMFILE" -f rawpcm-s16le -o "$BASE.hr" -g ti-hr

echo
echo Decoder Benchmark:
$GAPK -i "$BASE.fr"  -g rawpcm-s16le -o /dev/null -f gsm
$GAPK -i "$BASE.efr" -g rawpcm-s16le -o /dev/null -f amr-efr
$GAPK -i "$BASE.hr"  -g rawpcm-s16le -o /dev/null -f ti-hr
