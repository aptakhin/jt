Jt is codename for language, started for sandbox reasons.

Jt can do almost nothing, but my plans to make small language with tiny compiler/interpreter without garbage collector.

Jt requires Python 2.7 for development tools.

Jt uses gyp for generating project files.

Currently testing platform:

- Windows 7 x64, MSVS 13 upd. 1 x86 compiler

gyp generating:

	gyp jt.gyp --depth=proj/vs13 --generator-output=./proj/vs13 -f msvs -G msvs_version=2013
	
Nearest plans (to Dec 2014):

- more tests
- good compilation error reporting for user
- simple local type inferencer
- continue writing interpreter based on ast, slow but required
- starting Just-In-Time experiments

Next plans (to June 2015):

- stable language syntax, all features implemented for ast
- planning and implementing some core libraries