
#include "parser.h"
#include "ast-builder.h"

namespace jt {

Parser::Parser(FuncTermImpl* root, ContextSPtr ctx)
:	root_(root),
	ctx_(ctx),
	pstate_(yypstate_new(), [](yypstate* state) { yypstate_delete(state); }) {
	parse_ = std::make_unique<ParseContext>();
	parse_->root = root_;
	parse_->ctx  = ctx_;
}

void Parser::push(const String& c) {
	str_ += c;

	Lexer lexer(c.c_str(), c.c_str() + c.size(), line_, col_);
	int status = YYPUSH_MORE;

	do {
		YYSTYPE yylval;
		memset(&yylval, 0, sizeof(yylval));
		if (tokens_.empty() || (!tokens_.empty() && tokens_.back().lex != UNKNOWN))
			tokens_.push_back(Token());
		Token& tok = tokens_.back();
		lexer.next_lexeme(&tok);

		line_ = lexer.line();
		col_ = lexer.col();

		if (tok.lex == UNKNOWN)
			break;
		if (tok.lex == NEW_LINE) {
			line_ += 1; // TODO: More parser checks 
			col_ = 1;
			continue;
		}
		if (tok.lex == SPACE)
			continue; // TODO: More parser checks 
		int lexx = tok.lex;
		if (lexx == NUMBER)
			yylval.i = tok.i;
		if (!tok.ident.empty())
			yylval.str = (char*) tok.ident.c_str();
		status = yypush_parse(pstate_.get(), lexx, &yylval, parse_.get());
	} while (status == YYPUSH_MORE);
	parse_->finish();
}

} // namespace jt {