#pragma once

#include "common.h"
#include "ast.h"
#include "parser.h"
#include "runner.h"
#include <list>

namespace jt {

class ParseState {
public:
	ParseState();

	void push(Node v);
	void push_var(int number);
	void push_var(const String& n);

	Flow flow;

	FuncTermImpl* func;
	FuncCallImpl* func_call;
	VarImpl* var;
};

class ParseStates {
public:
	ParseStates();

	void push();
	void pop();
	void clear_flow();

	ParseState* operator -> ();

protected:
	std::list<ParseState> states_;
};

class ParseContext {
public:
	ParseContext();

	void def(const String& n);
	void def_end();

	void put_var(int number);
	void put_var(const String& n);

	void func_call(const String& name);
	void func_call_end();

	void put_func_call_all(const String& name, size_t args);

	void func_def(const String& name);
	void func_def_end();

	void func_def_param(const String& name, const String& type);
	void func_def_param_end(const char* rettype);

	FuncTermImpl* root;
	ContextSPtr   ctx;

	Node find_var(const String& name);

	void finish();

protected:
	ParseStates states_;
};

} // namespace jt {