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
	Lexer(const char* start, const char* end, int line, int col);

	void next_lexeme(Token* tok);

	int line() const { return line_; }
	int col() const { return col_; }

	bool finished() const;

private:
	char const* p;

	char const* const pe;
	char const* const eof;
	int cs;
	char const* ts;
	char const* te;
	int act;

	const char* orig_;
	int line_, col_;
	int offset_ = 0;
	const char* line_beg_;
};

} // namespace jt

