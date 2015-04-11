#include "lexer.h"
#include <cstdlib>
#include <string>

namespace jt {

%%{
	
machine Lexer;
alphtype char;
write data;

intlit = digit+;

ws = [ \t];

number = (intlit);
	
newline = ('\n');

action cmd_err {
	JT_TR("command error\n", PARSER_NOTIF); 
}
	
main := |*

	'def' => { tok->lex = DEF; fbreak; };

	'let' => { tok->lex = LET; fbreak; };

	number => {
		tok->lex = NUMBER;
		tok->i   = std::atoi(std::string(ts, te).c_str());
		fbreak;
	};

	'"'([a-zA-Z0-9_])+'"' => { 
		tok->ident = std::string(ts, te);
		tok->lex  = STR; fbreak; };

	[a-zA-Z_][a-zA-Z_0-9]* => { 
		tok->ident = std::string(ts, te);
		tok->lex  = IDENT; fbreak; };
	';'
		=> { tok->lex = SEMICOL; fbreak; };
	','
		=> { tok->lex = COMMA; fbreak; };
	'('
		=> { tok->lex = CIRC_OPEN; fbreak; };
	')'
		=> { tok->lex = CIRC_CLOSE; fbreak; };
	'{'
		=> { tok->lex = FIG_OPEN; fbreak; };
	'}'
		=> { tok->lex = FIG_CLOSE; fbreak; };
	'+'
		=> { tok->lex = PLUS; fbreak; };
	'-'
		=> { tok->lex = MINUS; fbreak; };
	'*'
		=> { tok->lex = MUL; fbreak; };
	'='
		=> { tok->lex = EQUAL; fbreak; };
	ws 
		=> { tok->lex = SPACE; fbreak; };

	newline => {
		line_beg_ = p + 1; // Skip new line character
		++line_;
		col_ = 1;
		tok->lex = NEW_LINE; 
		fbreak;
	};
*|;

}%%

Lexer::Lexer(const char* p_, const char* pe_, int line, int col)
:	p(p_),
	pe(pe_),
	eof(pe_),
	orig_(p_),
	line_(line),
	col_(col),
	line_beg_(orig_) {
	%% write init;
}

void Lexer::next_lexeme(Token* tok) {
	%% write exec;
	offset_ = p - orig_;
	tok->line = line_;
	tok->col  = col_;
	col_ = p - line_beg_ + 1;
}

bool Lexer::finished() const {
	return p == eof;
}

} // namespace jt {