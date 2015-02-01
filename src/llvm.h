// jt
//
#pragma once

#include "common.h"
#include "ast.h"

namespace jt {

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

	size_t counter_ = 0;
};

} // namespace jt {