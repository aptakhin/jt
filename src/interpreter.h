// jt
//
#pragma once

#include "ast.h"
#include "runner.h"
#include "lexer.h"
#include "ast-builder.h"
#include "inferencer.h"
#include "python-bind.h"
#include "llvm.h"
#include <gtest/gtest.h>

namespace jt {

std::vector<Node> listed(std::initializer_list<Node>&& init);

template <typename Ret, typename A1>
Term def_func(Var(*func)(CallUnit*, FuncTermImpl*, A1*), const String& a1name, Seq init_args = Seq()) {
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

	Term ret = make_term_move_ptr(func_impl);
	ret.set_abstract(false);
	return ret;
}

template <typename Ret, typename A1, typename A2>
Term def_func(Var(*func)(CallUnit*, FuncTermImpl*, A1*, A2*), const String& a1name, const String& a2name, Seq init_args = Seq()) {
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

	Term ret = make_term_move_ptr(func_impl);
	ret.set_abstract(false);
	return ret;
}

Term py_func(const String& module, const String& func, Seq args, Term ret_type);

class BaseEnv {
public:
	void setup_env();

	void setup_std(ContextSPtr ctx);

	ContextSPtr ctx_;
	std::unique_ptr<FuncTermImpl> root_;

	std::unique_ptr<CallUnit> run_;
	std::unique_ptr<Parser>   parser_;

	std::ostringstream out_;

	Assembly assembly_;
};

class BaseTest : public ::testing::Test, public BaseEnv {
protected:
	void SetUp() override;

	void TearDown() override;

	void exec();

	void call_print(const String& t_out);
};

class Interactive : private BaseEnv {
public:
	Interactive(std::istream& in);

	int exec(bool repl, const String& assembly);

protected:
	int exec_interactive();
	int exec_stream();

protected:
	std::istream& in_;
};

} // namespace jt {