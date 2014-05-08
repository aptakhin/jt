
#include "inferencer.h"

namespace jt {

Inferencer::Inferencer(FuncTermImpl& root, ContextSPtr ctx)
:	root_(root) {
	stack_.push_back(std::make_shared<Context>(ctx));
}

Term Inferencer::local(Node node) {
	switch (node->type()) {
	case NodeType::FLOW: {
		auto flow = node.impl<FlowImpl>();
		for (auto& i: flow->flow()) {
			auto term = local(i);
			if (!i->term())
				i->set_term(term);
		}
	}
		break;

	case NodeType::VAR: {
		auto n = node.impl<VarImpl>();
		auto tm = n->term();
		if (!n->term()) {
			auto set = n->value();
			tm = local(set);
			if (set->type() == NodeType::FUNC_CALL)
				set->set_term(tm);
			else
				n->set_term(tm);
		}

		if (auto func = n->term().if_is<FuncTermImpl>()) {
			if (n->term().is_abstract()) {
				// Setup input parameters, output
				auto flow = func->flow();
				auto term = local(flow);
			}
		}

		if (!n->name().empty())
			stack_.back()->add_named(n->name(), tm);
		return tm;
	}
		break;

	case NodeType::FUNC_CALL: {
		auto n = node.impl<FuncCallImpl>();
		Seq args;
		for (auto& i: n->flow()) {
			Term var_term = i->term();
			if (!i->term()) {
				var_term = local(i);
				if (!var_term)
					JT_COMP_ERR("Term wasn't evaluated");
				i->set_term(var_term);
			}
			args->add(make_var(var_term));
		}
		auto found = stack_.back()->find_named(n->name(), args);

		if (auto func = found.if_is<FuncTermImpl>()) {
			if (found.is_abstract()) {
				// Setup input parameters, output
				auto flow = func->flow();
				auto term = local(flow);
			}
		}

		if (found)
			return found;
	}
		break;
	}
	return Term();
}

} // namespace jt {