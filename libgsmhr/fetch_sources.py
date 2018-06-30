#!/usr/bin/env python

import urllib2
import os
import sys
import zipfile

try:
	import cStringIO as StringIO
except:
	import StringIO


SRC = "http://www.3gpp.org/ftp/Specs/archive/06_series/06.06/0606-421.zip"
HDR = {
	"User-Agent": "Mozilla/5.0 (X11; ArchLinux; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/60.0",
	"Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
}


def get_zipfile(data):
	return zipfile.ZipFile(StringIO.StringIO(data))


def get_subfile_data(data, filename):
	z = get_zipfile(data)
	return z.read(filename)


def process_file(z, e):
	fh = open(os.path.basename(e.filename.lower()), 'w')
	d = z.read(e).replace('\r','')
	fh.write(d)
	fh.close()


def main(*args):

	# Args
	if len(args) != 2:
		print "Usage: %s target_dir" % args[0]
		return

	tgt = args[1]

	# Create and go to target dir
	if not os.path.isdir(tgt):
		os.mkdir(tgt)
	os.chdir(tgt)

	# Get the original data
	req = urllib2.Request(SRC, headers = HDR)
	u = urllib2.urlopen(req)
	d = u.read()

	# Get DISK.zip
	d = get_subfile_data(d, 'DISK.zip')

	# Get Dir_C.zip
	d = get_subfile_data(d, 'Dir_C.zip')

	# Get zip file object
	z = get_zipfile(d)

	# Save each file
	for e in z.filelist:
		process_file(z, e)


if __name__ == '__main__':
	main(*sys.argv)
