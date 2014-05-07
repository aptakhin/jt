// jt
//
#pragma once

#include "common.h"
#include "ast-builder.h"
#include "lexer.h"
#include "runner.h"
#include "parser_gen.hpp"

namespace jt {

class Parser {
public:
	Parser(FuncTermImpl* root, ContextSPtr ctx);
	~Parser();

	void push(const String& c);
	const String& str() const { return str_; }

protected:
	FuncTermImpl* root_;
	ContextSPtr ctx_;

	yypstate* pstate_;

	String str_;
};

} // namespace jt {
