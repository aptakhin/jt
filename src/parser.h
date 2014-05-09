// jt
//
#pragma once

#include "common.h"
#include "lexer.h"
#include "runner.h"
#include "parser_gen.hpp"
#include <list>

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

	std::unique_ptr<ParseContext> parse_;

	String str_;

	std::list<Token> tokens_;
};

} // namespace jt {
