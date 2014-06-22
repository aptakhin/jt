
#include "ast.h"

namespace jt {

bool operator == (const TermType& a, const TermType& b) {
	return a.base() == b.base() && a.name() == b.name();
}

bool operator != (const TermType& a, const TermType& b) {
	return !(a == b);
}

void NodeImpl::visit(IVisitor* visitor) const {
	do_visit(visitor);
}

Term NodeImpl::term() {
	return term_;
}

const Term NodeImpl::term() const {
	return term_;
}

void NodeImpl::set_term(Term term) {
	term_ = term;
}

Term make_term_move_ptr(TermImpl* impl) {
	return Term(std::shared_ptr<TermImpl>(impl));
}

Term l_make_term(const char* type) {
	if (strcmp(type, "int") == 0)
		return make_term<IntTermImpl>();

	JT_COMP_ERR("Can't make term");
	return Term();
}

Node Node::clone() const {
	return Node(base_impl_->do_clone());
}

std::vector<Node>::iterator begin(Flow& f) {
	return f->flow().begin();
}

std::vector<Node>::iterator end(Flow& f) {
	return f->flow().end();
}

std::vector<Node>::const_iterator begin(const Flow& f) {
	return f->flow().begin();
}

std::vector<Node>::const_iterator end(const Flow& f) {
	return f->flow().end();
}

std::vector<Var>::iterator begin(Seq& s) {
	return s->vars().begin();
}

std::vector<Var>::iterator end(Seq& s) {
	return s->vars().end();
}

std::vector<Var>::const_iterator begin(const Seq& s) {
	return s->vars().begin();
}

std::vector<Var>::const_iterator end(const Seq& s) {
	return s->vars().end();
}

FlowImpl::FlowImpl()
:	NodeImpl(this) {}

FlowImpl::FlowImpl(FlowImpl&& flow)
:	NodeImpl(this),
	flow_(std::move(flow.flow_)) {}

FlowImpl::FlowImpl(const std::vector<Node>& nodes)
:	NodeImpl(this),
	flow_(nodes) {}

template <typename T, typename Container>
Container clone_container(const Container& cont) {
	Container res;
	res.reserve(cont.size());
	for (auto& i: cont)
		res.push_back(T(i->do_clone()));
	return std::move(res);
}

FlowImpl* FlowImpl::do_clone() const {
	auto flow   = new FlowImpl;
	flow->flow_ = clone_container<Node>(flow_);
	return flow;
}

void FlowImpl::do_visit(IVisitor* vis) const {
	vis->caption("Flow");
	for (auto& i: flow_)
		vis->visit("", i);
}

SeqImpl::SeqImpl()
:	NodeImpl(this) {}

SeqImpl::SeqImpl(const std::vector<Var>& vars)
:	NodeImpl(this),
	seq_(vars) {}

SeqImpl* SeqImpl::do_clone() const {
	auto seq  = new SeqImpl;
	seq->seq_ = clone_container<Var>(seq_);
	return seq;
}

Var SeqImpl::find(const String& name) const {
	for (auto& i: seq_) {
		if (i->name() == name)
			return i;
	}
	return Var();
}

void SeqImpl::do_visit(IVisitor* vis) const {
	vis->caption("Seq");
	for (auto& i: seq_)
		vis->visit("", i);
}

VarImpl* VarImpl::do_clone() const {
	auto var = new VarImpl();
	var->set_name(name_);
	if (value_) var->set_value(value_.clone());
	if (term()) var->set_term(term().clone());
	return var;
}

void VarImpl::do_visit(IVisitor* vis) const {
	vis->caption("Var ", name().c_str());
	if (term())
		vis->visit_term("Value", term());
	else if (value())
		vis->visit("Value", value());
}

FuncCallImpl* FuncCallImpl::do_clone() const {
	auto call = new FuncCallImpl(func_name_);
	call->set_flow(Flow(flow()->do_clone()));
	return call;
}

void FuncCallImpl::do_visit(IVisitor* vis) const {
	vis->caption("FuncCall ", name().c_str());
	for (auto& i: flow_)
		vis->visit("", i);
}

Term Term::clone() const {
	return make_term_move_ptr(impl_->do_clone());
}

TermImpl* Term::operator -> () {
	return impl_.get();
}

const TermImpl* Term::operator -> () const {
	return impl_.get();
}

Var make_ivar(int i) {
	Var v;
	v->set_term(make_term<IntTermImpl>(i));
	return v;
}

Var make_bvar(bool b) {
	Var v;
	v->set_term(make_term<BoolTermImpl>(b));
	return v;
}

Var make_svar(const String& str) {
	Var v;
	v->set_term(make_term<StringTermImpl>(str));
	return v;
}

Var make_var(Term term) {
	Var v;
	v->set_term(term);
	return v;
}

NativeFuncCallImpl* NativeFuncCallImpl::do_clone() const {
	auto func = new NativeFuncCallImpl(native_);
	func->set_args(Seq(args()->do_clone()));
	return func;
}

Var NativeFuncCallImpl::do_call(CallUnit* unit, FuncTermImpl* parent, Seq args) {
	return native_.call(unit, parent, args);
}

void NativeFuncCallImpl::do_visit(IVisitor* vis) const {
	vis->caption("NativeFuncCall");
}

FuncTermImpl* FuncTermImpl::do_clone() const {
	auto func = new FuncTermImpl;
	func->set_args(Seq(args()->do_clone()));
	func->set_flow(Flow(flow()->do_clone()));
	func->set_init_args(Seq(init_args()->do_clone()));
	func->set_ret(Var(ret()->do_clone()));
	return func;
}

IfImpl* IfImpl::do_clone() const {
	auto iff = new IfImpl;
	iff->set_cond(cond().clone());
	iff->set_then(then().clone());
	iff->set_other(other().clone());
	return iff;
}

void IfImpl::do_visit(IVisitor* visitor) const {

}

AstPrinter::AstPrinter(std::ostream& out)
:	out_(out),
	offset_(0) {}

void AstPrinter::caption(const char* caption) {
	print_offset();
	if (caption && caption[0] != 0)
		out_ << "Node: " << caption << std::endl;
}

void AstPrinter::caption(const char* title1, const char* title2) {
	print_offset();
	out_ << "Node: " << title1 << title2 << std::endl;
}

void AstPrinter::print_offset(int local) {
	for (int i = 0; i < std::max(0, offset_ + local); ++i)
		out_ << "\t";
}

void AstPrinter::visit(const char* title, const Node& node) {
	if (node.empty()) {
		print_offset();
		out_ << "{ EMPTY }" << std::endl;
	} else {
		print_offset();
		out_ << title << (!title || title[0] == 0 ? "" : " ") << "{" << std::endl;
		++offset_;
		node->visit(this);
		--offset_;
		print_offset();
		out_ << "}" << std::endl;
	}
}

void AstPrinter::visit_term(const char* title, const Term& term) {
	if (term.empty()) {
		print_offset();
		out_ << "{ EMPTY }" << std::endl;
	} else {
		if (auto f = term.as<FuncTermImpl>()) {
			out_ << std::endl;
			visit("Args", f->args());
			out_ << std::endl;
			visit("Flow", f->flow());
		} else {
			print_offset();
			out_ << title << (!title || title[0] == 0 ? "" : " ") << "{ ";
			if (auto n = term.as<IntTermImpl>())
				out_ << n->number();
			else if (auto s = term.as<StringTermImpl>())
				out_ << s->str();
			else
				out_ << "Can't print";
			out_ << " }" << std::endl;
		}
	}
}

AstChecker::AstChecker() {
	checks_.add([&](FuncTermImpl& func) {
		checks_.call(func.flow());
	});
}

void AstChecker::caption(const char* title) {}

void AstChecker::caption(const char* title1, const char* title2) {}

void AstChecker::visit(const char* title, const Node& node) {
	checks_.call(node);
}

void AstChecker::visit_term(const char* title, const Term& term) {
	checks_.call(term);
}

} // namespace jt {