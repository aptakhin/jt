
#include "common.h"
#include "ast.h"
#include "runner.h"
#include "lexer.h"
#include "ast-builder.h"
#include "inferencer.h"
#include "interpreter.h"
#include "llvm.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace jt;

#define TEST_OUT(t_out) { exec(); ASSERT_EQ(t_out, out_.str()); }

TEST_F(BaseTest, SimplePrint) {
	auto func_call = FuncCall("print", make_ivar(5));
	run_->set_flow(Flow(listed({func_call})));
	TEST_OUT("5");
}

TEST_F(BaseTest, SimplePlus) {
	auto op_call    = FuncCall("op_plus", make_ivar(2), make_ivar(3));
	auto print_call = FuncCall("print", op_call);
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("5");
}

TEST_F(BaseTest, SimpleVar) {
	auto op_call    = FuncCall("op_plus", make_ivar(2), make_ivar(3));
	Var pvar;
	pvar->set_value(op_call);
	auto print_call = FuncCall("print", pvar);
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("5");
}

TEST_F(BaseTest, SimpleEq) {
	auto cond = FuncCall("op_eq", make_ivar(2), make_ivar(2));
	Var pvar;
	pvar->set_value(cond);
	auto print_call = FuncCall("print", pvar);
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("true");
}

TEST_F(BaseTest, SimpleEqNot) {
	auto cond = FuncCall("op_eq", make_ivar(2), make_ivar(3));
	Var pvar;
	pvar->set_value(cond);
	auto print_call = FuncCall("print", pvar);
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("false");
}

TEST_F(BaseTest, SimplePrintStr) {
	auto print_call = FuncCall("print", make_svar("Test str"));
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("Test str");
}

TEST_F(BaseTest, SimpleIf) {
	auto cond = FuncCall("op_eq", make_ivar(2), make_ivar(3));
	auto iff  = If();
	iff->set_cond(cond);
	iff->set_then(FuncCall("print",  make_svar("Equal")));
	iff->set_other(FuncCall("print", make_svar("Not equal")));
	run_->set_flow(Flow(listed({iff})));
	TEST_OUT("Not equal");
}

TEST_F(BaseTest, SimpleIf2) {
	auto cond = FuncCall("op_eq", make_ivar(2), make_ivar(2));
	auto iff  = If();
	iff->set_cond(cond);
	iff->set_then(Flow(listed({
		FuncCall("print", make_svar("Equal")), 
		FuncCall("print", make_svar(" and smth else"))
	})));
	iff->set_other(FuncCall("print", make_svar("Not equal")));
	run_->set_flow(Flow(listed({iff})));
	TEST_OUT("Equal and smth else");
}

TEST(Common, Lexer) {
	char w[] = "def func()\n { x = 5; }";
	Lexer lexer(w, w + sizeof(w), 1, 1);

	while (true) {
		Token tok;
		lexer.next_lexeme(&tok);
		if (tok.lex == Lexeme::UNKNOWN)
			break;
	}
}

TEST_F(LexerTest, Numbers) {
	lex("x0z = 5;" 
		"x = 1 + 2;",
		{IDENT, EQUAL, NUMBER, SEMICOL, 
		IDENT, EQUAL, NUMBER, PLUS, NUMBER, SEMICOL}
	);
}

TEST_F(LexerTest, NumbersExps) {
	lex("x = 1 + -2;",
		{IDENT, EQUAL, NUMBER, PLUS, MINUS, NUMBER, SEMICOL}
	);
}

TEST_F(LexerTest, SimpleFunc) {
	lex("def func() {" 
		"  x = 5;" 
		"}",
		{DEF, IDENT, CIRC_OPEN, CIRC_CLOSE, FIG_OPEN, 
		IDENT, EQUAL, NUMBER, SEMICOL, 
		FIG_CLOSE}
	);
}

TEST_F(LexerTest, FuncInFunc) {
	lex("def func() {"
		"  def other(name int) int { name; }"
		"  x = 5;" 
		"}",
		{DEF, IDENT, CIRC_OPEN, CIRC_CLOSE, FIG_OPEN, 
		DEF, IDENT, CIRC_OPEN, IDENT, IDENT, CIRC_CLOSE, IDENT, FIG_OPEN, IDENT, SEMICOL, FIG_CLOSE, 
		IDENT, EQUAL, NUMBER, SEMICOL, 
		FIG_CLOSE}
	);
}

#include "parser.h"

TEST_F(BaseTest, Parser41) {
	parser_->push("x = 41;");
	call_print("x");
	TEST_OUT("41");
}

TEST_F(BaseTest, Parser41Circ) {
	parser_->push("x = (41);");
	call_print("x");
	TEST_OUT("41");
}

TEST_F(BaseTest, Parser41Plus1) {
	parser_->push("x = 41 + 1;");
	call_print("x");
	TEST_OUT("42");
}

TEST_F(BaseTest, Parser41Plus1Circ1) {
	parser_->push("x = (41 + 1);");
	call_print("x");
	TEST_OUT("42");
}

TEST_F(BaseTest, Parser41Plus1Circ2) {
	parser_->push("x = ((41) + (1));");
	call_print("x");
	TEST_OUT("42");
}

TEST_F(BaseTest, ParserMul) {
	parser_->push("x = 1 + 2 * 3;");
	call_print("x");
	TEST_OUT("7");
}

TEST_F(BaseTest, ParserMul2) {
	parser_->push("x = 1 * 2 + 3;");
	call_print("x");
	TEST_OUT("5");
}

TEST_F(BaseTest, ParserMul3) {
	parser_->push("x = 2 * (3 + 4);");
	call_print("x");
	TEST_OUT("14");
}

TEST_F(BaseTest, Parser42) {
	parser_->push("x = 41; y = x + 1;");
	call_print("y");
	TEST_OUT("42");
}

TEST_F(BaseTest, Parser43) {
	parser_->push("x = 41; y = x + 1; z = y + 1;");
	call_print("z");
	TEST_OUT("43");
}

TEST_F(BaseTest, Parser44) {
	parser_->push("x = 41; y = op_plus(x + 1, 2);");
	call_print("y");
	TEST_OUT("44");
}

TEST_F(BaseTest, DefFunc2) {
	parser_->push("def func() int { 2; } x = func();");
	call_print("x");
	TEST_OUT("2");
}

TEST_F(BaseTest, DefFunc4) {
	parser_->push("def func() int { 2 + 2; } x = func();");
	call_print("x");
	TEST_OUT("4");
}

TEST_F(BaseTest, DefFunc4Mul) {
	parser_->push("def func() int { 2 * 2; } x = func();");
	call_print("x");
	TEST_OUT("4");
}

TEST_F(BaseTest, DefFunc22) {
	parser_->push("def plus2(a int) int { a + 2; } x = plus2(20);");
	call_print("x");
	TEST_OUT("22");
}

TEST_F(BaseTest, DefFunc42Mul) {
	parser_->push("def double(a int) int { a * 2; } x = double(21);");
	call_print("x");
	TEST_OUT("42");
}

TEST_F(BaseTest, DefFunc12) {
	parser_->push("def plus2(a int) int { a + 2; } x = plus2(1) + plus2(2) + plus2(3);");
	call_print("x");
	TEST_OUT("12");
}

TEST_F(BaseTest, DefFunc12Circ) {
	parser_->push("def plus2(a int) int { a + 2; } x = (plus2(1) + ((plus2(2)) + plus2(3)));");
	call_print("x");
	TEST_OUT("12");
}

TEST_F(BaseTest, DefFunc42) {
	parser_->push("def func(a int) int { def plus2(b int) int { b + 2; } a + plus2(a); } x = func(20);");
	call_print("x");
	TEST_OUT("42");
}

TEST_F(BaseTest, DefFuncI) {
	parser_->push("def func(a) { a; } x = func(31);");
	call_print("x");
	TEST_OUT("31");
}

TEST_F(BaseTest, DefFuncI2) {
	parser_->push("def func(a) { a + 2; } x = func(31);");
	call_print("x");
	TEST_OUT("33");
}

TEST_F(BaseTest, DefFuncI3) {
	parser_->push("def func(a) { a * 2 + 1; } x = func(31);");
	call_print("x");
	TEST_OUT("63");
}

TEST_F(BaseTest, DISABLED_DefFuncI4) {
	parser_->push("def func(a) { a + 1; } x = 1 + func(31);");
	call_print("x");
	TEST_OUT("33");
}

TEST_F(BaseTest, DefFuncI5) {
	parser_->push("def func(a) { a * 3 + 1; } x = func(31 + 2);");
	call_print("x");
	TEST_OUT("100");
}

TEST_F(BaseTest, DefFuncDouble) {
	parser_->push("def func(a) { a + a; } x = func(2);");
	call_print("x");
	TEST_OUT("4");
}

TEST_F(BaseTest, DISABLED_DefFuncDoubleStr) {
	parser_->push("def func(a) { a + a; } x = func(\"Hello\");");
	call_print("x");
	TEST_OUT("HelloHello");
}
