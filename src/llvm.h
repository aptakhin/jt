// jt
//
#pragma once

#include "common.h"
#include "ast.h"

namespace jt {

class FormattedOutput {
public:
	FormattedOutput() = default;
	FormattedOutput(std::ostream& out);

	void open(std::ostream& out);

	FormattedOutput& print(const char* str);
	FormattedOutput& print(const char* str, size_t len);

	FormattedOutput& endl();

	FormattedOutput& operator << (const char* str);

private:
	void print_offset(int offset = 0);
	
private:
	std::ostream* out_ = nullptr;

	std::ostringstream line_;

	int offset_ = 0;
};

class Assembly {
public:
	Assembly();
	void open(const String& filename);

	void push(Node node);

private:
	void next(Node node, String& out);

	void g(const String& str);

	Assembly& operator << (const String& str);

	String tmp();
	String var(const String& name);

private:
	std::ofstream out_;

	FormattedOutput gen_;

	size_t counter_ = 0;
};

} // namespace jt {