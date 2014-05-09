
#include "ast.h"
#include "runner.h"
#include "lexer.h"
#include "ast-builder.h"
#include "inferencer.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace jt;

std::vector<Node> listed(std::initializer_list<Node>&& init) {
	return std::move(std::vector<Node>(init));
}

Var make_var_none() {
	return Var();
}

Var jt_print(CallUnit* unit, IntTermImpl* number) {
	std::ostringstream* out = (std::ostringstream*)
		unit->get_var("stream").impl<IntTermImpl>()->number();
	(*out) << number->number();
	return make_var_none();
}

Var jt_printb(CallUnit* unit, BoolTermImpl* b) {
	std::ostringstream* out = (std::ostringstream*)
		unit->get_var("stream").impl<IntTermImpl>()->number();
	(*out) << b->boolean();
	return make_var_none();
}

Var jt_prints(CallUnit* unit, StringTermImpl* s) {
	std::ostringstream* out = (std::ostringstream*)
		unit->get_var("stream").impl<IntTermImpl>()->number();
	(*out) << s->str();
	return make_var_none();
}

Var jt_plus(CallUnit*, IntTermImpl* a, IntTermImpl* b) {
	return make_ivar(a->number() + b->number());
}

Var jt_mul(CallUnit*, IntTermImpl* a, IntTermImpl* b) {
	return make_ivar(a->number() * b->number());
}

Var jt_eq(CallUnit*, IntTermImpl* a, IntTermImpl* b) {
	return make_bvar(a->number() == b->number());
}

Var jt_get(CallUnit* unit, StringTermImpl* varname) {
	return make_var(unit->get_var(varname->str()));
}

template <typename Ret, typename A1>
Term def_func(Var(*func)(CallUnit*, A1*), const String& a1name, Seq init_args = Seq()) {
	auto func_impl = new FuncTermImpl;
	{ // Filling arguments for function
		Seq seq;
		seq->add(make_typed_var<A1>(a1name, A1::TYPE));
		func_impl->set_args(seq);
		func_impl->set_init_args(init_args);
		func_impl->set_ret(make_typed_var<Ret>("", Ret::TYPE));
	}

	{ // Filling native call
		auto ncall = NativeFuncCall(NativeCall(func));
		Seq seq;
		seq->add(make_typed_var<A1>(a1name, A1::TYPE));
		ncall->set_args(seq);
		func_impl->set_flow(listed({ncall}));
	}

	return make_term_move_ptr(func_impl);
}

template <typename Ret, typename A1, typename A2>
Term def_func(Var(*func)(CallUnit*, A1*, A2*), const String& a1name, const String& a2name, Seq init_args = Seq()) {
	auto func_impl = new FuncTermImpl;
	{ // Filling arguments for function
		Seq seq;
		seq->add(make_typed_var<A1>(a1name, A1::TYPE));
		seq->add(make_typed_var<A2>(a2name, A2::TYPE));
		func_impl->set_args(seq);
		func_impl->set_init_args(init_args);
		func_impl->set_ret(make_typed_var<Ret>("", Ret::TYPE));
	}

	{ // Filling native call
		auto ncall = NativeFuncCall(NativeCall(func));
		Seq seq;
		seq->add(make_typed_var<A1>(a1name, A1::TYPE));
		seq->add(make_typed_var<A2>(a2name, A2::TYPE));
		ncall->set_args(seq);
		func_impl->set_flow(listed({ncall}));
	}

	return make_term_move_ptr(func_impl);
}

class BaseTest : public ::testing::Test {
protected:
	void SetUp() override {
		ctx_    = std::make_shared<Context>(nullptr);
		root_   = std::make_unique<FuncTermImpl>();
		run_    = std::make_unique<CallUnit>(nullptr, *root_.get(), ctx_);
		parser_ = std::make_unique<Parser>(root_.get(), ctx_);
		setup_std(ctx_);
		out_ << std::boolalpha;
	}

	void setup_std(ContextSPtr ctx) {
		Seq print_init_args;
		Var v = make_ivar((int) &out_);
		v->set_name("stream");
		print_init_args->add(v);
		ctx->add_named("print",   def_func<IntTermImpl>(jt_print,  "to_print", print_init_args));
		ctx->add_named("print",   def_func<IntTermImpl>(jt_printb, "to_print", print_init_args));
		ctx->add_named("print",   def_func<IntTermImpl>(jt_prints, "to_print", print_init_args));
		ctx->add_named("op_get",  def_func<IntTermImpl>(jt_get,    "var"));
		ctx->add_named("op_plus", def_func<IntTermImpl>(jt_plus,   "a", "b"));
		ctx->add_named("op_mul",  def_func<IntTermImpl>(jt_mul,    "a", "b"));
		ctx->add_named("op_eq",   def_func<BoolTermImpl>(jt_eq,    "a", "b"));
	}

#define TEST_OUT(t_out) { exec(); ASSERT_EQ(t_out, out_.str()); }

	void exec() {
		Inferencer inf(*root_.get(), ctx_);

		auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
		auto name = String() + "tests/" + test_info->name() + ".txt";

		std::ofstream fout(name.c_str());
		fout << parser_->str() << std::endl << std::endl;

		inf.local(run_->flow());

		AstPrinter print(fout);
		print.visit("", root_->flow());

		run_->exec();
	}

	void call_print(const String& t_out) {
		auto get   = FuncCall("op_get", make_svar(t_out));
		auto print = FuncCall("print", get);
		run_->flow()->add(print);
	}

protected:
	ContextSPtr ctx_;
	std::unique_ptr<FuncTermImpl> root_;

	std::unique_ptr<CallUnit> run_;
	std::unique_ptr<Parser>   parser_;

	std::ostringstream out_;
};

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
	auto print_call = FuncCall("print", make_svar("Test_str"));
	run_->set_flow(Flow(listed({print_call})));
	TEST_OUT("Test_str");
}

//TEST_F(BaseTest, SimpleIf) {
//	auto cond = FuncCall("op_eq", make_ivar(2), make_ivar(3));
//	auto iff  = If();
//	iff->set_cond(cond);
//	iff->set_then(FuncCall("print",  make_svar("equal")));
//	iff->set_other(FuncCall("print", make_svar("not_equal")));
//	run_->set_flow(Flow(listed({iff})));
//	TEST_OUT("not_equal");
//}

TEST(Common, Lexer) {
	char w[] = "def func()\n { x = 5; }";
	Lexer lexer(w, w + sizeof(w));

	while (true) {
		Token tok;
		lexer.next_lexeme(&tok);
		if (tok.lex == Lexeme::UNKNOWN)
			break;
	}
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

//// Precedence doesn't work
//TEST_F(BaseTest, ParserMul) {
//	parser_->push("x = 1 + 2 * 3;");
//	call_print("x");
//	TEST_OUT("7");
//}

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

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	auto out  = std::make_unique<OstreamReportOut>(std::cout);
	auto out2 = std::make_unique<Win32DbgReportOut>();
	Rep.add_out(out.get());
	Rep.add_out(out2.get());
	int result = RUN_ALL_TESTS();
	if (result != 0)
		int p = 0;
	return result;
}