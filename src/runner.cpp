
#include "runner.h"
#include "inferencer.h"
#include "python-bind.h"

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

Term Context::find_named(const String& name, Seq arg_types, Context** out_parent) {
	int best_score = 0;
	return find_best(name, arg_types, best_score, out_parent);
}

Term Context::find_best(const String& name, Seq arg_types, int& best_score, Context** out_parent) {
	Term best;
	auto r = terms_named_.equal_range(name);
	JT_TRACE_SCOPE("Searching " + name);
	for (auto i = r.first; i != r.second; ++i) {
		JT_TRACE_SCOPE("Checking");
		if (arg_types->empty()) {
			JT_COMP_ASSERT(best_score != 1, "Multiply names for empty query");
			best_score = 1;
			*out_parent = this;
			return i->second;
		}
		if (auto is = i->second.as<FuncTermImpl>()) {
			JT_TRACE_SCOPE("Test func suits");
			auto score = suits(is->args(), arg_types);
			if (score > best_score) {
				best = i->second;
				best_score = score;
				*out_parent = this;
			}
		}
	}
	if (parent_) {
		int before_score = best_score;
		auto parents = parent_->find_best(name, arg_types, best_score, out_parent);
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
		JT_TRACE_SCOPE(String() + "Test term type proto: " + type_name(i->term()) + "; input: " + type_name((*j)->term()));
		auto input = (*j)->term()->type();
		if (input == TermType::FUNC) {
			auto func = (*j)->term().impl<FuncTermImpl>();
			JT_COMP_ASSERT(func->ret()->term(), "No return term in func")
			input = func->ret()->term()->type();
		}
		if (i->term()) {
			if (i->term()->type() != input) {
				JT_TRACE("Type mismatch. Back")
				return 0;
			}
			suits_score += 2; // Exact match of types is preferred
		}
		else {
			suits_score += 1; // Match to generic type has little score
		}
		++j;
	}
	JT_TRACE("Score " + std::to_string(suits_score));
	return suits_score;
}

CallUnit::CallUnit(CallUnit* parent, FuncTermImpl& func, ContextSPtr ctx)
:	parent_(parent),
	func_(func) {
	stack_.push_back(std::make_shared<Context>(ctx));
}

void CallUnit::set_args(Seq args) {
	auto v = begin(args);
	JT_COMP_ASSERT(args->vars().size() == func_.args()->vars().size(), 
		"Calling function mismatched number of arguments");
	for (auto& arg: func_.args()) {
		auto x = resolve((*v));
		auto var_term = (*v)->term();
		stack_.back()->add_named(arg->name(), var_term);
		++v;
	}
}

void CallUnit::add_vars(Seq args) {
	for (auto& arg: args) {
		auto x = resolve(arg);
		auto var_term = arg->term();
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
	latest = exec_node(flow());
	return latest;
}

Var CallUnit::bind(Var var) {
	Context* parent_ctx = nullptr;
	auto found = stack_.back()->find_named(var->name(), Seq(), &parent_ctx);
	JT_COMP_ASSERT(found, "Name not found");
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
	Context* parent_ctx = nullptr;
	auto found = stack_.back()->find_named(name, Seq(), &parent_ctx);
	if (!found && parent_)
		return parent_->get_var(name);
	return found;
}

Var CallUnit::resolve(Node node) {
	auto lookup = node;
	if (auto v = node.as<VarImpl>()) {
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
	case NodeType::FLOW: {
		auto flow = node.impl<FlowImpl>();
		Term latest;
		for (auto& i: flow->flow())
			latest = exec_node(i);
		tm = latest;
	}
		break;

	case NodeType::FUNC_CALL: {
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

	case NodeType::NATIVE_FUNC_CALL: {
		auto call = node.impl<NativeFuncCallImpl>();
		auto push_args = bind(call->args());
		auto ret = call->do_call(this, &func_, push_args);
		if (ret->term())
			tm = ret->term();
	}
		break;

	case NodeType::PYTHON_FUNC_CALL: {
		auto call = node.impl<PythonFuncCallImpl>();
		auto push_args = bind(call->args());
		auto ret = call->do_call(this, &func_, push_args);
		if (ret->term())
			tm = ret->term();
	}
		break;

	case NodeType::VAR: {
		auto var  = node.impl<VarImpl>();
		auto calc = resolve(node);
		tm = calc->term();
		stack_.back()->add_named(var->name(), tm);
		return tm;
	}
		break;

	case NodeType::IF: {
		auto branch = node.impl<IfImpl>();
		stack_.push_back(std::make_shared<Context>(stack_.back()));
		auto condition = resolve(branch->cond());
		tm = condition->term();
		JT_COMP_ASSERT(tm->type() == TermType::BOOL, 
			"Inferencer error. Condition in if isn't boolean");
		if (auto cond = tm.as<BoolTermImpl>()) {
			Node next_exec = cond->boolean()? branch->then() : branch->other();
			if (next_exec) {
				// TODO: context stack push!
				tm = exec_node(next_exec);
			}	
		}
		stack_.pop_back();
		return tm;
	}
		break;
	}

	return tm;
}

} // namespace jt {