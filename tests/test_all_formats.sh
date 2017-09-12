#!/bin/sh


# This script generates test data for all formats by doing the
# following:
#  * encode the user-specified PCM file into each format
#  * re-decode that format back to PCM
#
# Use this to generate test files that are to be shipped together with
# gapk.   Always use the 


# directory in which the temporary output is stored
OUTDIR=/tmp

# name of the input s16le file to use for encoding and re-decoding
INFILE=$1

# source some common parameters
. ./common.sh

RETVAL=0

for f in $FORMATS; do
	BASE=`basename $INFILE`
	OUTFILE=$OUTDIR/$BASE.$f
	echo Format $f: Encoding $INFILE to $OUTFILE
	$GAPK -f rawpcm-s16le -i $INFILE -g $f -o $OUTFILE

	# compare with reference
	diff $OUTFILE $REFDIR/`basename $OUTFILE`
	if [ $? -ne 0 ]; then
		echo "===> FAIL"
		RETVAL=1
	else
		echo "===> PASS"
	fi
	echo

	DECFILE=$OUTFILE.s16
	echo Format $f: Decoding $OUTFILE to $DECFILE
	$GAPK -f $f -i $OUTFILE -g rawpcm-s16le -o $DECFILE

	# compare with reference
	diff $DECFILE $REFDIR/`basename $DECFILE`
	if [ $? -ne 0 ]; then
		echo "===> FAIL"
		RETVAL=1
	else
		echo "===> PASS"
	fi
	echo
done

echo -n "Overall Verdict: "
if [ $RETVAL -ne 0 ]; then
	echo "FAIL"
else
	echo "PASS"
fi

exit $RETVAL
