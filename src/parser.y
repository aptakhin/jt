%define api.pure
%define api.push_pull "push"

%code top 
{
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "lexer.h"
#include "ast-builder.h"

int yyerror(jt::ParseContext* ctx, char* s);
}

%code requires
{
namespace jt {
class ParseContext;
}
}

%code provides
{ 
#pragma once

#ifndef YYLTYPE
#define YYLTYPE YYLTYPE
typedef struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} YYLTYPE;

#endif
}

// Symbols.
%union
{
	char* str;
	int i;
};
%token UNKNOWN
%token SPACE
%token NEW_LINE
%token DEF
%token LET
%token SEMICOL
%token COMMA
%token CIRC_OPEN
%token CIRC_CLOSE
%token FIG_OPEN
%token FIG_CLOSE
%token EQUAL
%token PLUS
%token MUL
%token <str> IDENT
%token <str> STR
%token <i> NUMBER

%left PLUS
%left MUL 
%left EQUAL

%parse-param { jt::ParseContext* ctx };

%start Expressions
%%

Expressions:
	| Expressions Expr { 
		JT_TR("Next Expr", PARSER_NOTIF);
	}

Param:
	IDENT {
		JT_TR(jt::String() + "Par " + $1, PARSER_NOTIF);
		ctx->func_def_param($1, "");
	}
	| IDENT IDENT {
		JT_TR(jt::String() + "Par " + $1 + " " + $2, PARSER_NOTIF);
		ctx->func_def_param($1, $2);
	}

ParametersInt:
	| Param {
		JT_TR("Param", PARSER_NOTIF);
	}
	| ParametersInt COMMA Param {
		JT_TR("ParametersInt, Param", PARSER_NOTIF);
	}

Parameters:
	CIRC_OPEN ParametersInt CIRC_CLOSE {
		JT_TR("(ParametersInt)", PARSER_NOTIF);
	}

Returned:
	| IDENT {
		ctx->func_def_ret($1);
	}

TupleExprInt:
	| SubExpr {
		JT_TR("ParamExpr", PARSER_NOTIF);
	}
	| TupleExprInt COMMA SubExpr {
		JT_TR("ParamExprInt, ParamExpr", PARSER_NOTIF);
	}

TupleExpr:
	CIRC_OPEN TupleExprInt CIRC_CLOSE {
		JT_TR("(TupleExpr)", PARSER_NOTIF);
	}

FuncCall:
	IDENT {
		JT_TR("CallFunc: Ident", PARSER_NOTIF);
		ctx->func_call($1);
	}
	TupleExpr {
		JT_TR("CallFunc: (TupleExpr)", PARSER_NOTIF);
		ctx->func_call_end();
	}

AtomExpr:
	IDENT {
		JT_TR(jt::String() + "Ident " + $1, PARSER_NOTIF);
		ctx->put_ident($1);
	}
	| NUMBER {
		JT_TR(jt::String() + "Number " + std::to_string($1), PARSER_NOTIF);
		ctx->put_var($1);
	}
	| STR {
		JT_TR(jt::String() + "Str " + $1, PARSER_NOTIF);
		ctx->put_var($1);
	}
	| FuncCall {}
	| TupleExpr {
		JT_TR("(TupleExpr)", PARSER_NOTIF);
	}

SubExpr:
	AtomExpr {}
	| SubExpr PLUS SubExpr {
		JT_TR("SubExpr + AtomExpr", PARSER_NOTIF);
		ctx->put_func_call_all("op_plus", 2);
	}
	| SubExpr MUL SubExpr {
		JT_TR("SubExpr * AtomExpr", PARSER_NOTIF);
		ctx->put_func_call_all("op_mul", 2);
	}

FuncDef:
	DEF IDENT {
		JT_TR(jt::String() + "SubExpr " + $2 + " def start", PARSER_NOTIF);
		ctx->func_def($2);
	}
	Parameters { 
		JT_TR("Parameters", PARSER_NOTIF);
		ctx->func_def_param_end();
	} 
	Returned {
		JT_TR("Rettype", PARSER_NOTIF);
		ctx->func_def_ret_end();
	}
	FIG_OPEN {
		JT_TR("Func flow begin", PARSER_NOTIF);
	} 
	Expressions FIG_CLOSE {
		JT_TR("Func flow end", PARSER_NOTIF);
	}

DefineExpr: 
	IDENT EQUAL {
		JT_TR(jt::String() + "Define " + $1, PARSER_NOTIF);
		ctx->def($1);
	}
	SubExpr {
		ctx->def_end();
	}

Expr: 
	DefineExpr SEMICOL {}
	| FuncDef {
		JT_TR("Func def end;", PARSER_NOTIF);
		ctx->func_def_end();
	}
	| SubExpr SEMICOL {
		JT_TR("SubExpr;", PARSER_NOTIF);
	}
%%

int yyerror(jt::ParseContext* ctx, char* s) {
	JT_TR(jt::String() + "yyerror: " + s, PARSER_NOTIF);
	ctx->show_report(s);
	return 0;
}
