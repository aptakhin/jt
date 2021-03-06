#!/usr/bin/python
from __future__ import print_function
import sys

if sys.version_info.major >= 3:
	from urllib.request import urlopen
else:
	from urllib2 import urlopen

import os
from zipfile import ZipFile, BadZipfile

def install():
	arch_file = 'wintools.zip'

	print('downloading win32 tools')
	download_file('https://github.com/aptakhin/jt/releases/download/v0.0.0/wintools.zip', arch_file)

	try:
		with ZipFile(arch_file, 'r') as tools_zip:
			test = tools_zip.testzip()
			if test:
				print('archive corrupted, exit')
				exit()

			print('extracting')
			tools_zip.extractall('./')

		print('removing temporary archive')
		os.remove(arch_file) 

		print('it\'s ok, enjoy')

	except BadZipfile as e:
		print('can\'t open zip file: corrupted or something else, exit')
		exit()

def download_file(url, filename=None):
	if not filename:
		filename = url.split('/')[-1]

	u = urlopen(url)
	f = open(filename, 'wb')
	meta = u.info()
	file_size = int(meta.getheaders('Content-Length')[0])
	
	file_size_dl = 0
	block_sz = 8192
	while True:
		buffer = u.read(block_sz)
		if not buffer:
			break

		file_size_dl += len(buffer)
		f.write(buffer)
		status = r'%10d  [%3.2f%%]' % (file_size_dl, file_size_dl * 100. / file_size)
		status = status + chr(8) * (len(status) + 1)
		sys.stdout.write(status)

	f.close()
	print()

install()
