#!/bin/sh
OUTDIR=/tmp
INFILE=$1

. ./common.sh

for f in $FORMATS; do
	BASE=`basename $INFILE`
	PLAYFILE=$OUTDIR/$BASE.$f
	echo
	echo Format $f: Playing back $PLAYFILE
	echo $GAPK -f $f -i $PLAYFILE -g rawpcm-s16le -A default
	$GAPK -f $f -i $PLAYFILE -g rawpcm-s16le -A default
done
