
#include "ast.h"
#include "runner.h"
#include "lexer.h"
#include "ast-builder.h"
#include "inferencer.h"
#include "interpreter.h"
#include "llvm.h"
#include "python-bind.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace jt;

void help() {
	std::cout << "jt compiler/interpreter" << std::endl;
	std::cout << "Usage: " << std::endl;
	std::cout << "  jt [source]" << std::endl;
	std::cout << "Optional flags" << std::endl;
	std::cout << "  --run_internal_tests Run internal bundled tests and exit" << std::endl;
}

int main(int argc, char** argv) {
	String source;
	bool run_internal_tests = false;
	bool interactive = false;
	bool stream = false;
	String gen_llvm;

#if JT_PLATFORM_WIN32
	Win32DbgReportOut win32trace;
	Rep.add_out(&win32trace);
#endif

	//help();

	for (size_t i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--run_internal_tests") == 0)
			run_internal_tests = true;
		else 
		if (strcmp(argv[i], "-s") == 0)
			stream = true;
		else 
		if (strcmp(argv[i], "--gen_llvm") == 0)
			gen_llvm = argv[++i];
		else 
		if (i == argc - 1)
			source = argv[i];
	}
	PyEnv env;

	if (run_internal_tests) {
		testing::InitGoogleTest(&argc, argv);
		int result = RUN_ALL_TESTS();
		if (result != 0)
			JT_DBG_BREAK;
		//return result;
	}

	if (source.empty())
		interactive = true;

	if (interactive) {
		Interactive inter(std::cin);
		return inter.exec(stream? false : true, gen_llvm);
	}

	if (!source.empty()) {
		auto in = std::ifstream(source.c_str());
		Interactive inter(in);
		return inter.exec(false, gen_llvm);
	}

	return 0;
}