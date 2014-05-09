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
%token <i> NUMBER

%left PLUS
%left MUL 
%left EQUAL

%parse-param { jt::ParseContext* ctx };

%start Expressions
%%

Expressions:
	| Expressions Expr { 
		printf("Next Expr\n");
	}

Param:
	IDENT {
		printf("Par %s\n", $1);
		ctx->func_def_param($1, "");
	}
	| IDENT IDENT {
		printf("Par %s %s\n", $1, $2);
		ctx->func_def_param($1, $2);
	}

ParametersInt:
	| Param {
		printf("Param\n"); 
	}
	| ParametersInt COMMA Param {
		printf("ParametersInt, Param\n");
	}

Parameters:
	CIRC_OPEN ParametersInt CIRC_CLOSE {
		printf("(ParametersInt)\n"); 
	}

Returned:
	| IDENT {
		ctx->func_def_ret($1);
	}

TupleExprInt:
	| SubExpr {
		printf("ParamExpr\n"); 
	}
	| TupleExprInt COMMA SubExpr {
		printf("ParamExprInt, ParamExpr\n");
	}

TupleExpr:
	CIRC_OPEN TupleExprInt CIRC_CLOSE {
		printf("(TupleExpr)\n");
	}

FuncCall:
	IDENT {
		printf("CallFunc: Ident\n");
		ctx->func_call($1);
	}
	TupleExpr {
		printf("CallFunc: (TupleExpr)\n");
		ctx->func_call_end();
	}

AtomExpr:
	IDENT {
		printf("Ident %s\n", $1);
		ctx->put_var($1);
	}
	| NUMBER {
		printf("Number %d\n", $1);
		ctx->put_var($1);
	}
	| FuncCall {}
	| TupleExpr {
		printf("(TupleExpr)\n");
	}

SubExpr:
	AtomExpr {}
	| SubExpr PLUS SubExpr {
		printf("SubExpr + AtomExpr\n");
		ctx->put_func_call_all("op_plus", 2);
	}
	| SubExpr MUL SubExpr {
		printf("SubExpr * AtomExpr\n");
		ctx->put_func_call_all("op_mul", 2);
	}

FuncDef:
	DEF IDENT {
		printf("Func %s def start;\n", $2);
		ctx->func_def($2);
	}
	Parameters { 
		printf("Parameters\n");
		ctx->func_def_param_end();
	} 
	Returned {
		printf("Rettype\n");
		ctx->func_def_ret_end();
	}
	FIG_OPEN {
		printf("Func flow begin\n");
	} 
	Expressions FIG_CLOSE {
		printf("Func flow end\n");
	}

DefineExpr: 
	IDENT EQUAL {
		printf("Define %s:\n", $1);
		ctx->def($1);
	}
	SubExpr {
		ctx->def_end();
	}

Expr: 
	DefineExpr SEMICOL {}
	| FuncDef {
		printf("Func def end;\n");
		ctx->func_def_end();
	}
	| SubExpr SEMICOL {
		printf("SubExpr;\n");
	}
%%

int yyerror(jt::ParseContext* ctx, char* s)
{
	printf("yyerror: %s\n",s);
	ctx->show_report(s);
	return 0;
}
