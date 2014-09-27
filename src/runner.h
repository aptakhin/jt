// jt
//
#pragma once

#include "ast.h"
#include <map>

namespace jt {

class Context;
typedef std::shared_ptr<Context> ContextSPtr;
typedef std::vector<ContextSPtr> ContextStack;

class Context {
public:
	Context(ContextSPtr parent);

	void add(void* node, Term term);
	Optional<Term> find(void* node);

	void add_named(const String& name, Term term);
	Term find_named(const String& name, Seq arg_types, Context*& out_parent);

	int suits(Seq proto_types, Seq input_types);

protected:
	Term find_best(const String& name, Seq arg_types, int& score, Context*& out_parent);

protected:
	ContextSPtr parent_;

	typedef std::map<void*, Term> Terms;
	Terms terms_;

	typedef std::multimap<String, Term> TermsNamed;
	TermsNamed terms_named_;
};

class CallUnit {
public:
	CallUnit(CallUnit* parent, FuncTermImpl& func, ContextSPtr ctx);

	CallUnit(const CallUnit&) = delete;

	CallUnit(CallUnit&&);

	void set_args(Seq args);

	Flow flow();
	void set_flow(Flow flow);

	Term exec();

	Term get_var(const String& name);

private:
	void add_vars(Seq args);

	Term exec_node(Node node);

	Var resolve(Node node);
	Seq resolve(Flow flow);

	Var bind(Var var);
	Seq bind(Seq vars);

private:
	ContextStack stack_;

	CallUnit* parent_;

	FuncTermImpl& func_;
};

} // namespace jt {