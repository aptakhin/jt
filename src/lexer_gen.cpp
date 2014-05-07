
#line 1 "../src/lexer.rl"
#include "lexer.h"
#include <cstdlib>
#include <string>
#include <tchar.h>

namespace jt {


#line 9 "../src/lexer_gen.cpp"
static const char _Lexer_actions[] = {
	0, 1, 0, 1, 1, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 1, 14, 1, 
	15, 1, 16, 1, 17, 1, 18, 2, 
	2, 3, 2, 2, 4, 2, 2, 5
	
};

static const char _Lexer_key_offsets[] = {
	0, 0, 20, 22, 29, 36, 44, 52, 
	60
};

static const char _Lexer_trans_keys[] = {
	9, 10, 32, 40, 41, 43, 44, 59, 
	61, 95, 100, 108, 123, 125, 48, 57, 
	65, 90, 97, 122, 9, 32, 95, 48, 
	57, 65, 90, 97, 122, 95, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 102, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 116, 48, 57, 
	65, 90, 97, 122, 0
};

static const char _Lexer_single_lengths[] = {
	0, 14, 2, 1, 1, 2, 2, 2, 
	2
};

static const char _Lexer_range_lengths[] = {
	0, 3, 0, 3, 3, 3, 3, 3, 
	3
};

static const char _Lexer_index_offsets[] = {
	0, 0, 18, 21, 26, 31, 37, 43, 
	49
};

static const char _Lexer_indicies[] = {
	0, 2, 0, 3, 4, 5, 6, 8, 
	9, 10, 11, 12, 13, 14, 7, 10, 
	10, 1, 0, 0, 15, 10, 7, 10, 
	10, 16, 10, 10, 10, 10, 17, 10, 
	19, 10, 10, 10, 18, 10, 20, 10, 
	10, 10, 18, 10, 21, 10, 10, 10, 
	18, 10, 22, 10, 10, 10, 18, 0
};

static const char _Lexer_trans_targs[] = {
	2, 0, 1, 1, 1, 1, 1, 3, 
	1, 1, 4, 5, 7, 1, 1, 1, 
	1, 1, 1, 6, 4, 8, 4
};

static const char _Lexer_trans_actions[] = {
	0, 0, 21, 9, 11, 17, 7, 0, 
	5, 19, 37, 0, 0, 13, 15, 27, 
	23, 29, 25, 0, 31, 0, 34
};

static const char _Lexer_to_state_actions[] = {
	0, 1, 0, 0, 0, 0, 0, 0, 
	0
};

static const char _Lexer_from_state_actions[] = {
	0, 3, 0, 0, 0, 0, 0, 0, 
	0
};

static const char _Lexer_eof_trans[] = {
	0, 0, 16, 17, 18, 19, 19, 19, 
	19
};

static const int Lexer_start = 1;
static const int Lexer_first_final = 1;
static const int Lexer_error = 0;

static const int Lexer_en_main = 1;


#line 66 "../src/lexer.rl"


Lexer::Lexer(const char* p_, const char* pe_)
:	p(p_),
	pe(pe_),
	eof(pe_),
	orig_(p_),
	line_beg_(orig_) {
	
#line 101 "../src/lexer_gen.cpp"
	{
	cs = Lexer_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 75 "../src/lexer.rl"
}

void Lexer::next_lexeme(Token* tok) {
	
#line 110 "../src/lexer_gen.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _Lexer_actions + _Lexer_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 1:
#line 1 "NONE"
	{ts = p;}
	break;
#line 129 "../src/lexer_gen.cpp"
		}
	}

	_keys = _Lexer_trans_keys + _Lexer_key_offsets[cs];
	_trans = _Lexer_index_offsets[cs];

	_klen = _Lexer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _Lexer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _Lexer_indicies[_trans];
_eof_trans:
	cs = _Lexer_trans_targs[_trans];

	if ( _Lexer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _Lexer_actions + _Lexer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 2:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 3:
#line 24 "../src/lexer.rl"
	{act = 1;}
	break;
	case 4:
#line 26 "../src/lexer.rl"
	{act = 2;}
	break;
	case 5:
#line 34 "../src/lexer.rl"
	{act = 4;}
	break;
	case 6:
#line 38 "../src/lexer.rl"
	{te = p+1;{ tok->lex = SEMICOL; {p++; goto _out; } }}
	break;
	case 7:
#line 40 "../src/lexer.rl"
	{te = p+1;{ tok->lex = COMMA; {p++; goto _out; } }}
	break;
	case 8:
#line 42 "../src/lexer.rl"
	{te = p+1;{ tok->lex = CIRC_OPEN; {p++; goto _out; } }}
	break;
	case 9:
#line 44 "../src/lexer.rl"
	{te = p+1;{ tok->lex = CIRC_CLOSE; {p++; goto _out; } }}
	break;
	case 10:
#line 46 "../src/lexer.rl"
	{te = p+1;{ tok->lex = FIG_OPEN; {p++; goto _out; } }}
	break;
	case 11:
#line 48 "../src/lexer.rl"
	{te = p+1;{ tok->lex = FIG_CLOSE; {p++; goto _out; } }}
	break;
	case 12:
#line 50 "../src/lexer.rl"
	{te = p+1;{ tok->lex = PLUS; {p++; goto _out; } }}
	break;
	case 13:
#line 52 "../src/lexer.rl"
	{te = p+1;{ tok->lex = EQUAL; {p++; goto _out; } }}
	break;
	case 14:
#line 57 "../src/lexer.rl"
	{te = p+1;{
		line_beg_ = p + 1; // Skip new line character
		++line_;
		col_ = 1;
		tok->lex = NEW_LINE; 
		{p++; goto _out; }
	}}
	break;
	case 15:
#line 28 "../src/lexer.rl"
	{te = p;p--;{
		tok->lex = NUMBER;
		tok->i   = std::atoi(std::string(ts, te).c_str());
		{p++; goto _out; }
	}}
	break;
	case 16:
#line 34 "../src/lexer.rl"
	{te = p;p--;{ 
		tok->ident = std::string(ts, te);
		tok->lex  = IDENT; {p++; goto _out; } }}
	break;
	case 17:
#line 53 "../src/lexer.rl"
	{te = p;p--;{
		tok->lex = SPACE; {p++; goto _out; }
	}}
	break;
	case 18:
#line 1 "NONE"
	{	switch( act ) {
	case 1:
	{{p = ((te))-1;} tok->lex = DEF; {p++; goto _out; } }
	break;
	case 2:
	{{p = ((te))-1;} tok->lex = LET; {p++; goto _out; } }
	break;
	case 4:
	{{p = ((te))-1;} 
		tok->ident = std::string(ts, te);
		tok->lex  = IDENT; {p++; goto _out; } }
	break;
	}
	}
	break;
#line 272 "../src/lexer_gen.cpp"
		}
	}

_again:
	_acts = _Lexer_actions + _Lexer_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 0:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 283 "../src/lexer_gen.cpp"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _Lexer_eof_trans[cs] > 0 ) {
		_trans = _Lexer_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 79 "../src/lexer.rl"
	offset_ = p - orig_;
	tok->line = line_;
	tok->col  = col_;
	col_ = p - line_beg_;
}

} // namespace jt {