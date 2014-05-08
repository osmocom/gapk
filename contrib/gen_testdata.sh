#!/bin/sh

# script to generate a .s16 file (s16le with 8kHz) from an arbitrary MP3
# usage: gen_testdata.sh foo.mp3 foo.s16

WAV=`mktemp`
MP3=$1
S16=$1

mpg123 -w "$WAV" "$MP3"
sox "$WAV" -b 16 -c 1 -r 8000 "$S16"
rm "$WAV"
