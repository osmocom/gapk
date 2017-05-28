#!/bin/sh

# This script re-generates the reference data shipped together with
# gapk. To be used by gapk maintainers only.

. ./common.sh

INFILE=$1
OUTDIR=$REFDIR


for f in $FORMATS; do
	BASE=`basename $INFILE`
	OUTFILE=$OUTDIR/$BASE.$f
	echo
	echo Format $f: Encoding $INFILE to $OUTFILE
	$GAPK -f rawpcm-s16le -i $INFILE -g $f -o $OUTFILE

	DECFILE=$OUTFILE.s16
	echo Format $f: Decoding $OUTFILE to $DECFILE
	$GAPK -f $f -i $OUTFILE -g rawpcm-s16le -o $DECFILE
done
