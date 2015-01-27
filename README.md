Jt is small sandbox static-typed language without garbage collector. So, welcome to reinvents powered language!

## Example

	def double(a) {
		a + a
	}

Calculating `а` type when use ```double```:

	x = double(3)
	> 6
	y = double("ab")
	> "abab"

Also, imagine the great power of 4 basis types: integer, asci string, boolean and function!

## Compilation and other humdrum

Requires Python 2.7 or new 3.* already installed for development i.e. some install stuff and GYP project generator. And also seems not very bad C++11 compiler is a requirement too.

Code is almost cross platform, except some debugging features. Currently testing platform:

- Windows 7 x64, MSVS 13 upd. 1 x86 compiler (Earlier compiler versions are not supported)

Call ```install.py``` to setup required tools on Windows:

- GYP
- Ragel
- bison
- LLVM assembler and interpreter

Install them manually on other platforms or stay tuned!

### Hints

Use GYP to generate project files for your favourite environment/IDE:

	gyp jt.gyp --depth=proj/vs13 --generator-output=./proj/vs13 -f msvs -G msvs_version=2013

	
Regenerating lexer (Ragel) and parser (bison) files after changing grammar:	
	
	cd wintools
	gen.bat
