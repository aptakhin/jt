Jt is codename for language started for sandbox reasons. Jt's slogan: "Reinvents powered".

Jt can do almost nothing, but my plans to make small language with tiny compiler/interpreter without garbage collector.

Jt requires Python 2.7 for development tools. Jt uses gyp for generating project files.

Currently testing platform:

- Windows 7 x64, MSVS 13 upd. 1 x86 compiler (Earlier compiler versions are not supported)

gyp generating:

	gyp jt.gyp --depth=proj/vs13 --generator-output=./proj/vs13 -f msvs -G msvs_version=2013
	
Now (to Dec 2014):

- more tests
- good compilation error reporting for user
- simple local type inferencer
- continue writing interpreter based on ast, slow but required
- starting Just-In-Time and native compilation experiments

Then (to June 2015):

- stable language syntax, all features implemented for ast
- plans and implementations of some core libraries
