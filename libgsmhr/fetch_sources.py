#!/usr/bin/env python3

import logging as log
import urllib.request
import io
import os
import sys
import zipfile


URL = "http://www.3gpp.org/ftp/Specs/archive/06_series/06.06/0606-421.zip"


def get_zipfile(data: bytes) -> zipfile.ZipFile:
	return zipfile.ZipFile(io.BytesIO(data), 'r')


def get_subfile_data(data: bytes, filename: str):
	log.debug('Unpacking \'%s\'', filename)
	z = get_zipfile(data)
	return z.read(filename)


def process_file(z, e):
	log.debug('Processing file \'%s\'', e.filename.lower())
	fh = open(os.path.basename(e.filename.lower()), 'wb')
	d = z.read(e).replace(b'\r', b'')
	fh.write(d)
	fh.close()


def main(*args):

	# Args
	if len(args) != 2:
		print("Usage: %s target_dir" % args[0])
		return

	tgt = args[1]

	# Create and go to target dir
	if not os.path.isdir(tgt):
		os.mkdir(tgt)
	os.chdir(tgt)

	# Get the original data
	log.info('Requesting file: %s', URL)
	with urllib.request.urlopen(URL) as response:
		log.debug('Response code: %d', response.code)
		assert response.code == 200

		for h in ('Last-Modified', 'Content-Type', 'Content-Length'):
			log.debug('%s: %s', h, response.getheader(h))

		log.info('Downloading %d bytes...', response.length)
		d = response.read()

	# Get DISK.zip
	d = get_subfile_data(d, 'DISK.zip')

	# Get Dir_C.zip
	d = get_subfile_data(d, 'Dir_C.zip')

	# Get zip file object
	z = get_zipfile(d)

	# Save each file
	for e in z.filelist:
		process_file(z, e)


log.basicConfig(format='[%(levelname)s] %(filename)s:%(lineno)d %(message)s', level=log.DEBUG)

if __name__ == '__main__':
	main(*sys.argv)
