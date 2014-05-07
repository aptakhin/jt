#include "parser.h"
#include "parser_gen.hpp"
#include <list>

namespace jt {

Parser::Parser(FuncTermImpl* root, ContextSPtr ctx)
	: root_(root),
	ctx_(ctx),
	pstate_(yypstate_new()) {}

Parser::~Parser() {
	yypstate_delete(pstate_);
}

void Parser::push(const String& c) {
	str_ += c;

	Lexer lexer(c.c_str(), c.c_str() + c.size());
	int status;

	ParseContext parse;
	parse.root = root_;
	parse.ctx = ctx_;
	int lexx;
	std::list<Token> tokens;
	do {
		YYSTYPE yylval;
		memset(&yylval, 0, sizeof(yylval));
		tokens.push_back(Token());
		Token& tok = tokens.back();
		lexer.next_lexeme(&tok);

		if (tok.lex == UNKNOWN)
			break;
		if (tok.lex == SPACE)
			continue; // TODO: More parser checks 
		lexx = tok.lex;
		if (lexx == NUMBER)
			yylval.i = tok.i;
		if (!tok.ident.empty())
			yylval.str = (char*) tok.ident.c_str();
		status = yypush_parse(pstate_, lexx, &yylval, &parse);
	} while (status == YYPUSH_MORE);
	parse.finish();
}

} // namespace jt {