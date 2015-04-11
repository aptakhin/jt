#include "ast-builder.h"

namespace jt {

ParseState::ParseState()
:	func(nullptr),
	func_call(nullptr),
	var(nullptr) {}

void ParseState::push(Node node) {
	JT_TR(jt::String() + "-- ADD " + std::to_string(int(node->type())), PARSER_NOTIF);
	flow->add(node);
}

void ParseState::push_var(int n) {
	flow->add(make_ivar(n));
}

void ParseState::push_var(const char* str) {
	flow->add(make_svar(str));
}

void ParseState::push_ident(const char* id) {
	auto get = FuncCall("op_get", make_svar(id));
	flow->add(get);
}

ParseStates::ParseStates() {
	push();
}

ParseState* ParseStates::operator -> () {
	return &states_.back();
}

void ParseStates::push() {
	JT_TR(jt::String() + "-- PUSH " + std::to_string(states_.size()), PARSER_NOTIF);
	states_.push_back(ParseState());
}

void ParseStates::pop() {
	states_.pop_back();
	JT_TR( "-- POP", PARSER_NOTIF);
}

void ParseStates::clear_flow() {
	JT_TR( "-- CLEAR", PARSER_NOTIF);
	states_.back().flow->flow().clear();
}

void ParseContext::show_report(const char* s) {
	String format = s;
	jt::Rep.report(jt::Report(jt::ReportLevel::USER_ERR, __FILE__, __LINE__, format.c_str())); 
}

void ParseContext::put_var(int number) {
	states_->push_var(number);
}

void ParseContext::put_var(const char* str) {
	states_->push_var(str);
}

void ParseContext::put_ident(const char* id) {
	states_->push_ident(id);
}

void ParseContext::func_call(const String& name) {
	states_.push();
	states_->func_call = new FuncCallImpl(name);
	states_.push();
}

void ParseContext::func_call_end() {
	auto moved = std::move(states_->flow);
	states_.pop();
	auto func_call = FuncCall(states_->func_call);
	func_call->set_flow(moved);
	states_.pop();
	states_->flow->add(func_call);
}

void ParseContext::put_func_call_all(const String& name, size_t args) {
	Flow set_args;
	// This all is hacking, because thinking in push/pop became too difficult
	// Cut last _args_ nodes and push them to new call.
	auto i = states_->flow->flow().begin();
	JT_COMP_ASSERT(states_->flow->flow().size() >= args, 
		"Can't cut such quantity of nodes from flow");
	std::advance(i, states_->flow->flow().size() - args);
	auto erase_from = i;
	for (size_t j = 0; j < args; ++i, ++j)
		set_args->add(*i);
	states_->flow->flow().erase(erase_from, states_->flow->flow().end());

	states_->func_call = new FuncCallImpl(name);
	auto func_call = FuncCall(states_->func_call);
	func_call->set_flow(set_args);
	states_->flow->add(func_call);
}

void ParseContext::func_def(const String& name) {
	states_.push();
	states_->func = new FuncTermImpl;
	states_->func->type().set_name(name);
	states_->var  = new VarImpl;
	states_->var->set_term(make_term_move_ptr(states_->func));
	states_->var->set_name(name);
	states_.push();
}

void ParseContext::func_def_end() {
	auto moved = std::move(states_->flow);
	states_.pop();
	states_->func->set_flow(moved);
	auto& new_func = *states_->var;
	states_.pop();
	states_->flow->add(Var(new_func));
}

void ParseContext::def(const String& name) {
	states_.push();
	states_->var = new VarImpl;
	states_->var->set_name(name);
	states_.push();
}

void ParseContext::def_end() {
	if (states_->flow->flow().size() != 1)
		JT_COMP_ERR("Only one subnode can be left for define node");
	auto setval = states_->flow->flow()[0].base_impl();
	states_.pop();
	auto move_var = states_->var;
	move_var->set_value(setval);
	states_.pop();
	states_->flow->add(Var(move_var));
}

Seq seq_from_flow(Flow flow) {
	Seq ret;
	for (auto& i: flow)
		ret->add(Var((VarImpl*) i.base_impl()));
	return ret;
}

void ParseContext::func_def_param(const char* name, const char* type) {
	Var v;
	v->set_name(name);
	if (type[0] != 0)
		v->set_term(l_make_term(type));
	states_->push(v);
}

void ParseContext::func_def_param_end() {
	auto moved = seq_from_flow(states_->flow);
	states_.pop();
	states_->func->set_args(moved);
	states_.push();
}

void ParseContext::func_def_ret(const char* type) {
	Var v;
	v->set_term(l_make_term(type));
	states_->push(v);
}

void ParseContext::func_def_ret_end() {
	auto moved = seq_from_flow(states_->flow);
	states_.pop();
	JT_COMP_ASSERT(moved->vars().size() <= 1, "More than one return type isn't supported yet");
	if (!moved->vars().empty())
		states_->func->set_ret(moved->vars().front());
	states_.push();
}

void ParseContext::finish() {
	root->set_flow(std::move(states_->flow));
}

} // namespace jt {