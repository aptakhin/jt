{
'includes': [
	'jt-common.gypi'
],
'targets': [
{
	'target_name': 'jt',
	'type': 'executable',
	'sources': [
		'src/ast-builder.cpp',
		'src/ast-builder.h',
		'src/ast.cpp',
		'src/ast.h',
		'src/common.cpp',
		'src/common.h',
		'src/gtest/gtest-all.cc',
		'src/gtest/gtest.h',
		'src/inferencer.cpp',
		'src/inferencer.h',
		'src/lexer.h',
		'src/lexer.rl',
		'src/lexer_gen.cpp',
		'src/parser.cpp',
		'src/parser.h',
		'src/parser.y',
		'src/parser_gen.cpp',
		'src/parser_gen.hpp',
		'src/report.cpp',
		'src/report.h',
		'src/runner.cpp',
		'src/runner.h',
		'src/test.cpp',
	],
	'include_dirs': [
		'src/'
	],
},
],
}
