// jt
//
#pragma once

#include "common.h"
#include "parser_gen.hpp"

namespace jt {

typedef enum yytokentype Lexeme;

struct Token {
	Lexeme lex = Lexeme::UNKNOWN;
	int i = 0;
	std::string ident;

	int line = 1, col = 1;
};

class Lexer {
public:
	Lexer(const char* start, const char* end);

	void next_lexeme(Token* tok);

private:
	char const* p;

	char const* const pe;
	char const* const eof;
	int cs;
	char const* ts;
	char const* te;
	int act;

	const char* orig_;
	int line_ = 0, col_ = 0;
	int offset_ = 0;
	const char* line_beg_;
};

} // namespace jt

