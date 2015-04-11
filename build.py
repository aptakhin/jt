#!/usr/bin/python
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
		print >>out, self.name + ': ' + ' '.join(self.deps)
		for cmd in self.cmds:
			print >>out, '\t' + cmd

class MakefileGenerator:
	def __init__(self, out):
		self.out = out
		pass

dst_dir = 'build/clang-3.7'
dst_obj_dir = os.path.join(dst_dir, 'objs')
if not os.path.exists(dst_obj_dir):
    os.makedirs(dst_obj_dir)

def cxx_target(filename):
	basename = os.path.basename(filename)
	name, ext = os.path.splitext(basename)
	name = os.path.join(dst_obj_dir, name + '.o')
	#print name, ext

	cmd = MkCommand(name)
	cmd.deps.append(filename)
	cmd.cmds.append('$(CXX) $(CPPFLAGS) -c {} -o {}'.format(filename, cmd.name))
	return cmd

with open('Makefile', 'wt') as makefile:
	print >>makefile, '''# built by builder
CXX=clang++
RM=rm -f'''

	print >>makefile, 'CPPFLAGS=-std=c++14 -ferror-limit=1 -Wall -pedantic -I src -I', '/opt/local/Library/Frameworks/Python.framework/Versions/3.4/Headers'
	source_files = source.split('\n')
	procs = []
	for source in source_files:
		source = source.strip()
		if not source:
			continue
		cmd = cxx_target(source)
		procs.append(cmd)

	print >>makefile, 'all: jt'
	print >>makefile, ''

	print >>makefile, 'jt:', ' '.join((i.name for i in procs)), 'lexer', 'parser'
	print >>makefile, '\t$(CXX) $(CPPFLAGS) -g -Wall -pedantic -v -o', os.path.join(dst_dir, 'jt'), ' '.join((i.name for i in procs))#, '-l/opt/local/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4/config-3.4m/libpython3.4.dylib'

	print >>makefile, 'lexer: src/lexer_gen.cpp'
	print >>makefile, '\t'

	print >>makefile, 'src/lexer_gen.cpp: src/lexer.rl'
	print >>makefile, '\t' + 'ragel -o src/lexer_gen.cpp src/lexer.rl'

	print >>makefile, 'parser: src/parser_gen.cpp'
	print >>makefile, '\t'

	print >>makefile, 'src/parser_gen.cpp: src/parser.y src/parser.h'
	print >>makefile, '\t' + 'bison -d -o src/parser_gen.cpp src/parser.y'
	print >>makefile, ''

	print >>makefile, 'clean:'
	print >>makefile, '\t' + 'rm -r', dst_dir
	print >>makefile, ''
	for i in procs:
		i.write(makefile)
		print >>makefile, ''
