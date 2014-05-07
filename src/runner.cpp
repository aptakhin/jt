
#include "runner.h"

namespace jt {

Context::Context(ContextSPtr parent)
:	parent_(parent) {}

void Context::add(void* node, Term term) {
	terms_.insert(std::make_pair(node, term));
}

Optional<Term> Context::find(void* node) {
	return iter2optional<Terms>(terms_.find(node), terms_.end());
}

void Context::add_named(const String& name, Term term) {
	terms_named_.insert(std::make_pair(name, term));
}

Term Context::find_named(const String& name, Seq arg_types) {
	int best_score = 0;
	return find_best(name, arg_types, best_score);
}

Term Context::find_best(const String& name, Seq arg_types, int& best_score) {
	Term best;
	auto r = terms_named_.equal_range(name);
	for (auto i = r.first; i != r.second; ++i) {
		if (arg_types->empty()) {
			if (best_score == 1)
				JT_COMP_ERR("Multiply names for empty query");
			best_score = 1;
			return i->second;
		}
		if (auto is = i->second.if_is<FuncTermImpl>()) {
			auto score = suits(is->args(), arg_types);
			if (score > best_score) {
				best = i->second;
				best_score = score;
			}
		}
	}
	if (parent_) {
		int before_score = best_score;
		Term parents = parent_->find_best(name, arg_types, best_score);
		if (best_score > before_score)
			best = parents;
	}
	return best;
}

int Context::suits(Seq proto_types, Seq input_types) {
	if (proto_types->vars().size() != input_types->vars().size())
		return 0;

	int suits_score = 0;
	auto j = begin(input_types);
	for (auto& i: proto_types) {
		auto proto = i->term()->type();
		auto input = (*j)->term()->type();
		if (input == TermType::FUNC) {
			auto func = (*j)->term().impl<FuncTermImpl>();
			input = func->ret()->term()->type();
		}
		if (proto == input)
			++suits_score;
		else if (proto != TermType::ABSTRACT)
			return 0;
		++j;
	}
	return suits_score;
}

CallUnit::CallUnit(CallUnit* parent, FuncTermImpl& func, ContextSPtr ctx)
:	parent_(parent),
	func_(func) {
	stack_.push_back(std::make_shared<Context>(ctx));
}

CallUnit::CallUnit(CallUnit&& mv)
:	stack_(std::move(mv.stack_)),
	parent_(mv.parent_),
	func_(std::move(mv.func_)) {}

void CallUnit::set_args(Seq args) {
	auto v = begin(args);
	for (auto& arg: func_.args()) {
		assert(v != end(args));
		auto x = resolve((*v));
		Term var_term = (*v)->term();
		stack_.back()->add_named(arg->name(), var_term);
		++v;
	}
}

void CallUnit::add_vars(Seq args) {
	for (auto& arg: args) {
		auto x = resolve(arg);
		Term var_term = arg->term();

		stack_.back()->add_named(arg->name(), var_term);
	}
}

Flow CallUnit::flow() {
	return func_.flow();
}

void CallUnit::set_flow(Flow flow) {
	func_.set_flow(flow);
}

Term CallUnit::exec() {
	Term latest;
	for (auto& i: flow())
		latest = exec_node(i);
	return latest;
}

Var CallUnit::bind(Var var) {
	auto found = stack_.back()->find_named(var->name(), Seq());
	if (!found)
		JT_COMP_ERR("Name not found");
	Var ret;
	ret->set_name(var->name());
	ret->set_term(found);
	return ret;
}

Seq CallUnit::bind(Seq vars) {
	Seq ret;
	for (auto& i: vars)
		ret->add(bind(i));
	return ret;
}

Term CallUnit::get_var(const String& name) {
	auto found = stack_.back()->find_named(name, Seq());
	if (!found && parent_)
		return parent_->get_var(name);
	return found;
}

Var CallUnit::resolve(Node node) {
	auto lookup = node;
	if (auto v = node.if_is<VarImpl>()) {
		if (v->term()) {
			if (!v->value() || v->term()->type() != TermType::FUNC)
				return Var(v);
		}

		lookup = v->value();
	}

	if (lookup) {
		auto term = exec_node(lookup);
		if (!term || term->type() == TermType::FUNC)
			JT_COMP_ERR("Error in resolving type in CallUnit::resolve (Missing term)");
		auto var = make_var(term);
		return var;
	}

	JT_COMP_ERR("Can't resolve such type in CallUnit::resolve");
	return Var();
}

Seq CallUnit::resolve(Flow flow) {
	Seq res;
	for (auto& i: flow)
		res->add(resolve(i));
	return res;
}

Term CallUnit::exec_node(Node node) {
	Term tm;

	if (node->term())
		tm = node->term();
	else {
		auto found = stack_.back()->find(node.base_impl());
		if (found)
			tm = *found;
	}
	switch (node->type()) {
	case NodeType::FUNC_CALL:
	{
		auto call = node.impl<FuncCallImpl>();
		auto impl = tm.impl<FuncTermImpl>();
		JT_COMP_ASSERT(impl, "No function term impl in function call");
		CallUnit next(this, *impl, nullptr);
		next.set_flow(impl->flow());
		next.add_vars(impl->init_args());
		Seq args = resolve(call->flow());
		next.set_args(args);
		tm = next.exec();
	}
		break;

	case NodeType::NATIVE_FUNC_CALL:
	{
		auto call = node.impl<NativeFuncCallImpl>();
		auto push_args = bind(call->args());
		auto ret = call->do_call(this, push_args);
		if (ret->term())
			tm = ret->term();
	}
		break;

	case NodeType::VAR:
	{
		auto n = node.impl<VarImpl>();
		auto calc = resolve(node);
		tm = calc->term();
		stack_.back()->add_named(n->name(), tm);
		return tm;
	}
		break;
	}

	return tm;
}

} // namespace jt {