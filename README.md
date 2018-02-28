Introduction
============
The G SM A udio P ocket K nive (GAPK) is a small command-line tool that supports conversion between the various GSM related codecs (HR/FR/EFR/AMR) and PCM audio. It supports many different formats for the codec frames, including ETSI and IETF standardized formats, as well as vendor-specific formats like those found in the TI Calypso DSP (see OsmocomBB) and those of Racal 6103/6113 GSM test equipment

Installation
============
1. Clone the repository
2. Run autoreconf --install
3. Run ./configure
4. Run make install

Example Usage
=============
Do osmo-gapk -h for help

osmo-gapk -i result123.gsm -f rtp-hr-ietf -A default -g rawpcm-s16le

1. result123.gsm is input file
2. rtp-hr-ietf is input file format
3. -A default selects default ALSA audio sink, usually your speakers
4. -g is output format




