#include "parser.h"
#include "ast-builder.h"

namespace jt {

Parser::Parser(FuncTermImpl* root, ContextSPtr ctx)
:	root_(root),
	ctx_(ctx),
	pstate_(yypstate_new()) {
	parse_ = std::make_unique<ParseContext>();
	parse_->root = root_;
	parse_->ctx  = ctx_;
}

Parser::~Parser() {
	yypstate_delete(pstate_);
}

void Parser::push(const String& c) {
	str_ += c;

	Lexer lexer(c.c_str(), c.c_str() + c.size());
	int status;

	do {
		YYSTYPE yylval;
		memset(&yylval, 0, sizeof(yylval));
		tokens_.push_back(Token());
		Token& tok = tokens_.back();
		lexer.next_lexeme(&tok);

		if (tok.lex == UNKNOWN)
			break;
		if (tok.lex == SPACE || tok.lex == NEW_LINE)
			continue; // TODO: More parser checks 
		int lexx = tok.lex;
		if (lexx == NUMBER)
			yylval.i = tok.i;
		if (!tok.ident.empty())
			yylval.str = (char*) tok.ident.c_str();
		status = yypush_parse(pstate_, lexx, &yylval, parse_.get());
	} while (status == YYPUSH_MORE);
	parse_->finish();
}

} // namespace jt {