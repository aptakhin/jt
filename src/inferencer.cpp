
#include "inferencer.h"

namespace jt {

Inferencer::Inferencer(FuncTermImpl& root, ContextSPtr ctx)
:	root_(root) {
	stack_.push_back(std::make_shared<Context>(ctx));
}

Term Inferencer::local(Node node) {
	JT_TRACE_SCOPE("Local inference at node");
	switch (node->type()) {
	case NodeType::FLOW: {
		JT_TRACE_SCOPE("Flow");
		auto flow = node.impl<FlowImpl>();
		for (auto& i: flow->flow()) {
			auto term = local(i);
			if (!i->term())
				i->set_term(term);
		}
	}
		break;

	case NodeType::SEQ: {
		JT_TRACE_SCOPE("Seq");
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
		JT_TRACE_SCOPE("Var: " + var->name());
		auto tm  = var->term();
		if (!var->term()) {
			JT_TRACE_SCOPE("Inferencing call");
			auto set = var->value();
			tm = local(set);
			if (set->type() == NodeType::FUNC_CALL)
				set->set_term(tm);
			else
				var->set_term(tm);
		}

		if (auto func = var->term().as<FuncTermImpl>()) {
			if (var->term().is_abstract()) {
				JT_TRACE_SCOPE("Inference var from function call");
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
		JT_TRACE_SCOPE("FuncCall: " + call->name());
		Seq args;
		JT_TRACE_SCOPE("Inference calls flow");
		for (auto& i: call->flow()) {
			Term var_term = i->term();
			JT_TRACE_SCOPE("Call for node: " + type_name(i));
			if (!var_term) {
				JT_TRACE_SCOPE("Evaluating");
				var_term = local(i);
				if (!var_term)
					JT_COMP_ERR("Term wasn't evaluated");
				i->set_term(var_term);
			}
			JT_TRACE("Term type: " + type_name(var_term));
			args->add(make_var(var_term));
		}
		Context* parent_ctx = nullptr;
		auto found = stack_.back()->find_named(call->name(), args, parent_ctx);

		if (auto func = found.as<FuncTermImpl>()) {
			if (found.is_abstract()) {
				// It's function with generic types
				// Clone whole function content and setup with arguments called.
				auto specialized = func->do_clone();
				auto found = make_term_move_ptr(specialized);
				found.set_abstract(false);

				for (auto i: zip(args->vars(), specialized->args()->vars())) {
					auto set = i.fst->impl()->term().clone();
					i.snd->impl()->set_term(set);
				}

				parent_ctx->add_named(call->name(), found);

				// Setup input parameters, output
				Inferencer inf(*specialized, nullptr);
				inf.local(args); // Pass args
				inf.local(specialized->flow());
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

String type_name(Term term) {
	std::ostringstream out;
	switch (term.type().base()) {
	case TermType::BOOL: {
		return "Bool";
		//out << "(" << std::boolalpha << term.as<BoolTermImpl>()->boolean() << ")";
	}

	case TermType::STRING:
		return "String";
		//out << "(" << term.as<StringTermImpl>()->str() << ")";

	case TermType::FUNC:
		return "Func";
		//auto func = term.as<FuncTermImpl>();
		//out << traceable_print(func->init_args());
		//out << traceable_print(func->flow());
		return out.str();
	}

	return "Unknown";
}

String type_name(Node node) {
	switch (node.type()) {
	case NodeType::FLOW:
		return "Flow";
	case NodeType::FUNC:
		return "Func";
	case NodeType::FUNC_CALL:
		return "FuncCall";
	case NodeType::IF:
		return "If";
	case NodeType::NATIVE_FUNC_CALL:
		return "NativeFuncCall";
	case NodeType::SEQ:
		return "Seq";
	case NodeType::VAR:
		return "Var";
	}

	return "Unknown";
}

} // namespace jt {