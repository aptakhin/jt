#!/usr/bin/python
from __future__ import print_function
import os

source = '''
src/ast-builder.cpp
src/ast.cpp
src/common.cpp
src/gtest/gtest-all.cc
src/inferencer.cpp
src/interpreter.cpp
src/llvm.cpp
src/lexer_gen.cpp
src/parser.cpp
src/parser_gen.cpp
src/python-bind.cpp
src/report.cpp
src/runner.cpp
src/test.cpp
src/main.cpp
'''

class MkCommand:
	def __init__(self, name):
		self.name = name
		self.deps = []
		self.cmds = []

	def write(self, out):
		print(self.name + ': ' + ' '.join(self.deps), file=out)
		for cmd in self.cmds:
			print('\t' + cmd, file=out)

class MakefileGenerator:
	def __init__(self, out):
		self.out = out
		pass

if __name__ == '__main__':
	py_include_dir = '/opt/local/Library/Frameworks/Python.framework/Versions/3.4/Headers'
	py_lib = '/opt/local/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4/config-3.4m/libpython3.4.a'

	dst_dir = 'proj/clang-3.7'
	dst_obj_dir = os.path.join(dst_dir, 'objs')

	if not os.path.exists(dst_obj_dir):
	    os.makedirs(dst_obj_dir)

	def cxx_target(filename):
		basename = os.path.basename(filename)
		name, ext = os.path.splitext(basename)
		name = os.path.join(dst_obj_dir, name + '.o')

		cmd = MkCommand(name)
		cmd.deps.append(filename)
		cmd.cmds.append('$(CXX) $(CPPFLAGS) -c {} -o {}'.format(filename, cmd.name))
		return cmd

	with open('Makefile', 'wt') as makefile:
		print('# built by builder', file=makefile)
		print('CXX=clang++', file=makefile)
		print('RM=rm -f', file=makefile)

		print('CPPFLAGS=-std=c++14 -ferror-limit=1 -Wall -pedantic -I src -I', py_include_dir, file=makefile)
		source_files = source.split('\n')
		procs = []
		for source in source_files:
			source = source.strip()
			if not source:
				continue
			cmd = cxx_target(source)
			procs.append(cmd)

		print('all: jt', file=makefile)
		print('', file=makefile)

		print('jt:', ' '.join((i.name for i in procs)), 'lexer', 'parser', file=makefile)
		print('\t$(CXX) $(CPPFLAGS) -g -Wall -pedantic -v -o', os.path.join(dst_dir, 'jt'), ' '.join((i.name for i in procs)), py_lib, file=makefile)

		print('lexer: src/lexer_gen.cpp', file=makefile)
		print('\t', file=makefile)

		print('src/lexer_gen.cpp: src/lexer.rl', file=makefile)
		print('\t' + 'ragel -o src/lexer_gen.cpp src/lexer.rl', file=makefile)

		print('parser: src/parser_gen.cpp', file=makefile)
		print('\t', file=makefile)

		print('src/parser_gen.cpp: src/parser.y src/parser.h', file=makefile)
		print('\t' + 'bison -d -o src/parser_gen.cpp src/parser.y', file=makefile)
		print('', file=makefile)

		print('clean:', file=makefile)
		print('\t' + 'rm -r', dst_dir, file=makefile)
		print('', file=makefile)
		for i in procs:
			i.write(makefile)
			print('', file=makefile)
