
#include "ast.h"

namespace jt {

bool operator == (const TermType& a, const TermType& b) {
	return a.base() == b.base();
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

void SeqImpl::do_visit(IVisitor* vis) const {
	vis->caption("Seq");
	for (auto& i: seq_)
		vis->visit("", i);
}

void VarImpl::do_visit(IVisitor* vis) const {
	vis->caption("Var ", name().c_str());
	if (term())
		vis->visit_term("Value", term());
	else if (value())
		vis->visit("Value", value());
}

void FuncCallImpl::do_visit(IVisitor* vis) const {
	vis->caption("FuncCall ", name().c_str());
	for (auto& i: flow_)
		vis->visit("", i);
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

Var NativeFuncCallImpl::do_call(CallUnit* unit, Seq args) {
	return native_.call(unit, args);
}

void NativeFuncCallImpl::do_visit(IVisitor* vis) const {
	vis->caption("NativeFuncCall");
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
		if (auto f = term.if_is<FuncTermImpl>()) {
			out_ << std::endl;
			visit("Args", f->args());
			out_ << std::endl;
			visit("Flow", f->flow());
		} else {
			print_offset();
			out_ << title << (!title || title[0] == 0 ? "" : " ") << "{ ";
			if (auto n = term.if_is<IntTermImpl>())
				out_ << n->number();
			else if (auto s = term.if_is<StringTermImpl>())
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