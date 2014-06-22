
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

	case NodeType::SEQ: {
		auto seq = node.impl<SeqImpl>();
		for (auto& i: seq->vars()) {
			auto term = local(i);
			if (!i->term())
				i->set_term(term);
		}
	}
		break;

	case NodeType::VAR: {
		auto var = node.impl<VarImpl>();
		auto tm  = var->term();
		if (!var->term()) {
			auto set = var->value();
			tm = local(set);
			if (set->type() == NodeType::FUNC_CALL)
				set->set_term(tm);
			else
				var->set_term(tm);
		}

		if (auto func = var->term().as<FuncTermImpl>()) {
			if (var->term().is_abstract()) {
				// Setup input parameters, output
				auto flow = func->flow();
				auto term = local(flow);
			}
		}

		if (!var->name().empty())
			stack_.back()->add_named(var->name(), tm);
		return tm;
	}
		break;

	case NodeType::FUNC_CALL: {
		auto call = node.impl<FuncCallImpl>();
		Seq args;
		for (auto& i: call->flow()) {
			Term var_term = i->term();
			if (!i->term()) {
				var_term = local(i);
				if (!var_term)
					JT_COMP_ERR("Term wasn't evaluated");
				i->set_term(var_term);
			}
			args->add(make_var(var_term));
		}
		auto found = stack_.back()->find_named(call->name(), args);

		if (auto func = found.as<FuncTermImpl>()) {
			if (found.is_abstract()) {
				// It's function with generic types
				auto specialized = func->do_clone();
				// Cloning all function and setup with arguments called.
				auto found = make_term_move_ptr(specialized);
				found.set_abstract(false);

				for (auto i: zip(args->vars(), specialized->args()->vars())) {
					auto set = i.fst->impl()->term().clone();
					i.snd->impl()->set_term(set);
				}

				stack_.back()->add_named(call->name(), found);

				// Setup input parameters, output
				Inferencer inf(*specialized, nullptr);
				inf.local(args); // Pass args
				inf.local(specialized->flow());
				auto flow = func->flow();
				auto term = local(flow);
			}
		}

		if (found)
			return found;
	}
		break;

	case NodeType::IF: {
		auto iff = node.impl<IfImpl>();
		auto cond_term  = local(iff->cond());
		iff->cond()->set_term(cond_term);
		auto then_term  = local(iff->then());
		iff->then()->set_term(then_term);
		auto other_term = local(iff->other());
		iff->other()->set_term(other_term);
	}
		break;
	}
	return Term();
}

} // namespace jt {