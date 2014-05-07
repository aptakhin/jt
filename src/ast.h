// jt
//
#pragma once

#include "common.h"
#include <vector>

namespace jt {

class Term;
class Node;
class NodeImpl;
class CallUnit;

class BadNodeCast : public std::logic_error {
public:
	BadNodeCast(const char* what) : std::logic_error(what) { _CrtDbgBreak(); }
};

class Node {
public:
	Node(NodeImpl* impl) : base_impl_(impl) {}
	virtual ~Node() {}

	NodeImpl* base_impl() { return base_impl_; }

	bool empty() const { return base_impl_ == nullptr; }

	operator bool() const { return !empty(); }

	NodeImpl* operator ->() {
		return base_impl_;
	}

	const NodeImpl* operator ->() const {
		return base_impl_;
	}

	template <typename Impl>
	bool is() const {
		return base_impl_ && base_impl_->type() == Impl::TYPE;
	}

	template <typename Impl>
	Impl* if_is() {
		return is<Impl>() ? impl<Impl>() : nullptr;
	}

	template <typename Impl>
	const Impl* if_is() const {
		return is<Impl>() ? impl<Impl>() : nullptr;
	}

	template <typename Impl>
	Impl* impl() {
		assert(is<Impl>());
		return reinterpret_cast<Impl*>(base_impl_);
	}

protected:
	NodeImpl* base_impl_ = nullptr;
};

template <class T>
class AstNodeGuard : public Node {
public:
	AstNodeGuard(T* node) : Node(node), node_(node) {}

	T* operator ->() {
		return node_;
	}

	const T* operator ->() const {
		return node_;
	}

	T* impl() {
		return node_;
	}

	const T* impl() const {
		return node_;
	}

private:
	T* node_ = nullptr;
};

#define JT_CAT_IMPL_(A, B) A ## B
#define JT_CAT(A, B) JT_CAT_IMPL_(A, B)

#define JT_AST_NODE(Name) \
class  JT_CAT(Name, Impl);\
class Name : public AstNodeGuard< JT_CAT(Name, Impl) >\
{\
public:\
template <typename... Args>\
Name(Args... args) : AstNodeGuard(new JT_CAT(Name, Impl)(args...)) {}\
\
Name(JT_CAT(Name, Impl)* impl) : AstNodeGuard(impl) {}\
Name(Name&& mv) : AstNodeGuard(mv.impl()) {}\
/*Name(const Name& cpy) : AstNodeGuard(const_cast<JT_CAT(Name, Impl)*>(cpy.impl())) {}*/\
/*template <>\
Name(const std::initializer_list<Node>& init) : AstNodeGuard(new JT_CAT(Name, Impl)(init)) {}*/\
};\
class JT_CAT(Name, Impl) : public NodeImpl

enum class NodeType {
	UNKNOWN,
	FLOW,
	SEQ,
	VAR,
	FUNC,
	FUNC_CALL,
	NATIVE_FUNC_CALL
};

class Term;

class TermType {
public:
	enum Type {
		UNKNOWN,
		ABSTRACT,
		INT4,
		STRING,
		FUNC,
	};

	TermType(Type type) : type_(type) {}

	Type base() const { return type_; }

private:
	Type type_;
};

bool operator == (const TermType& a, const TermType& b);
bool operator != (const TermType& a, const TermType& b);

class TermImpl {
public:
	template <typename Derived>
	TermImpl(Derived* derived)
		: type_(Derived::TYPE) {
		JT_COMP_ASSERT((void*) derived == (TermImpl*) derived,
			"Term derived type can't be sliced");
	}

	virtual ~TermImpl() {}

	TermType type() const { return type_; }

protected:
	TermType type_;
};

class AbstractTermImpl : public TermImpl {
public:
	static const TermType::Type TYPE = TermType::ABSTRACT;

	AbstractTermImpl(const String& abstract_type) : TermImpl(this), abstract_type_(abstract_type) {}

	const String& abstract_type() const { return abstract_type_; }

private:
	const String abstract_type_;
};

class IntTermImpl : public TermImpl {
public:
	static const TermType::Type TYPE = TermType::INT4;

	IntTermImpl(int number = 0) : TermImpl(this), number_(number) {}

	int number() const { return number_; }

private:
	int number_;
};

class StringTermImpl : public TermImpl {
public:
	static const TermType::Type TYPE = TermType::STRING;

	StringTermImpl(const String& str = "") : TermImpl(this), str_(str) {}

	const String& str() const { return str_; }

private:
	const String str_;
};

class Term {
public:
	Term() = default;

	Term(TermType type) : type_(type) {}

	Term(std::shared_ptr<TermImpl> impl) : impl_(impl), type_(impl_->type()) {}

	Term clone();

	TermImpl* operator -> ();
	const TermImpl* operator -> () const;

	template <typename Impl>
	bool is() const {
		return impl_.get() && impl_->type().base() == Impl::TYPE;
	}

	template <typename Impl>
	Impl* if_is() {
		return is<Impl>() ? impl<Impl>() : nullptr;
	}

	template <typename Impl>
	const Impl* if_is() const {
		return is<Impl>() ? impl<Impl>() : nullptr;
	}

	template <typename Impl>
	Impl* impl() {
		if (!is<Impl>()) JT_COMP_ERR("Bad term cast");
		return reinterpret_cast<Impl*>(impl_.get());
	}

	template <typename Impl>
	const Impl* impl() const {
		if (!is<Impl>()) JT_COMP_ERR("Bad term cast");
		return reinterpret_cast<const Impl*>(impl_.get());
	}

	const TermType type() const { return type_; }

	bool empty() const { return impl_.get() == nullptr; }

	operator bool() const { return !empty(); }

	bool is_abstract() const { return abstract_; }
	void set_abstract(bool abstract) { abstract_ = abstract; }

private:
	std::shared_ptr<TermImpl> impl_;

	TermType type_ = TermType::UNKNOWN;

	bool abstract_ = true;
};

template <typename TermImplem, typename... Args>
Term make_term(Args... args) {
	return Term(std::make_shared<TermImplem>(args...));
}

Term make_term_move_ptr(TermImpl* impl);

Term l_make_term(const char* type);

class Seq;
class Flow;

class IVisitor {
public:
	virtual ~IVisitor() {}

	virtual void caption(const char* title) = 0;
	virtual void caption(const char* title1, const char* title2) = 0;
	virtual void visit(const char* title, const Node& node) = 0;
	virtual void visit_term(const char* title, const Term& term) = 0;
};

class NodeImpl {
public:
	template <typename Derived>
	NodeImpl(Derived* derived)
	:	type_(Derived::TYPE) {
		JT_COMP_ASSERT((void*) derived == (NodeImpl*) derived,
			"Node derived type can't be sliced");
	}

	virtual ~NodeImpl() {}

	Term term();
	const Term term() const;
	void set_term(Term term);

	NodeType type() const { return type_; }

	void visit(IVisitor* visitor) const;

protected:
	virtual void do_visit(IVisitor* visitor) const = 0;

private:
	NodeType type_;

	Term term_;
};

JT_AST_NODE(Var) {
public:
	static const NodeType TYPE = NodeType::VAR;

	VarImpl() : NodeImpl(this) {}

	void set_name(const String& name) { name_ = name; }
	const String& name() const { return name_; }

	Node value() const { return value_; }
	void set_value(Node node) { value_ = node; }

	virtual void do_visit(IVisitor* visitor) const override;

private:
	String name_;

	Node value_ = nullptr;
};

Var make_ivar(int i);
Var make_svar(const String& str);

template <typename TermImpl>
Var make_typed_var(const String& name, TermType type) {
	Var v;
	v->set_name(name);
	v->set_term(make_term<TermImpl>());
	return v;
}

Var make_var(Term term);

JT_AST_NODE(Flow) {
public:
	static const NodeType TYPE = NodeType::FLOW;

	FlowImpl();
	FlowImpl(FlowImpl&& flow);
	FlowImpl(const std::vector<Node>& nodes);

	std::vector<Node>&       flow() { return flow_; }
	const std::vector<Node>& flow() const { return flow_; }

	void add(Node node) { flow_.push_back(node); }

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	std::vector<Node> flow_;
};

std::vector<Node>::iterator begin(Flow& f);
std::vector<Node>::iterator end(Flow& f);

std::vector<Node>::const_iterator begin(const Flow& f);
std::vector<Node>::const_iterator end(const Flow& f);

JT_AST_NODE(Seq) {
public:
	static const NodeType TYPE = NodeType::SEQ;

	SeqImpl();
	SeqImpl(const std::vector<Var>& vars);

	std::vector<Var>&       vars() { return seq_; }
	const std::vector<Var>& vars() const { return seq_; }

	bool empty() const { return seq_.empty(); }

	void add(Var var) { seq_.push_back(var); }

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	std::vector<Var> seq_;
};

std::vector<Var>::iterator begin(Seq& s);
std::vector<Var>::iterator end(Seq& s);

std::vector<Var>::const_iterator begin(const Seq& s);
std::vector<Var>::const_iterator end(const Seq& s);

class Modifier {
public:
	Modifier(const String& name, Seq args)
	:	name_(name),
		args_(args) {}

protected:
	const String name_;
	Seq args_;
};

class TemplateMod : public Modifier {
public:
	TemplateMod(Seq args)
	:	Modifier("Template", args) {}
};

JT_AST_NODE(FuncCall) {
public:
	static const NodeType TYPE = NodeType::FUNC_CALL;

	FuncCallImpl() : NodeImpl(this) {}

	FuncCallImpl(const String& func_name)
	:	NodeImpl(this),
		func_name_(func_name) {}

	FuncCallImpl(const String& func_name, Node n1)
	:	NodeImpl(this),
		func_name_(func_name) {
		flow_->add(n1);
	}

	FuncCallImpl(const String& func_name, Node n1, Node n2)
		: NodeImpl(this),
		func_name_(func_name) {
		flow_->add(n1);
		flow_->add(n2);
	}

	const String& name() const { return func_name_; }
	Flow flow() { return flow_; }
	void set_flow(Flow flow) { flow_ = flow; }

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	const String func_name_;
	Flow flow_;
};

typedef Var(*JtNative) (Seq);

template <typename A1>
Var func_map_seq(void* func, CallUnit* unit, Seq args) {
	assert(args->vars().size() == 1);
	typedef Var(*NativeFunc) (CallUnit*, A1*);
	NativeFunc native_func = (NativeFunc) func;
	auto a0 = args->vars()[0]->term().impl<A1>();
	return native_func(unit, a0);
}

template <typename A1, typename A2>
Var func_map_seq(void* func, CallUnit* unit, Seq args) {
	assert(args->vars().size() == 2);
	typedef Var(*NativeFunc) (CallUnit*, A1*, A2*);
	NativeFunc native_func = (NativeFunc) func;
	auto a0 = args->vars()[0]->term().impl<A1>();
	auto a1 = args->vars()[1]->term().impl<A2>();
	return native_func(unit, a0, a1);
}

class NativeCall {
public:
	typedef Var(*NativeMapper) (void* func, CallUnit* unit, Seq args);
	typedef void(*Store)(...);

	NativeCall()
	:	call_(nullptr), native_func_(nullptr) {}

	template <typename A1>
	NativeCall(Var(*native_func)(CallUnit* unit, A1*))
	:	call_(&func_map_seq<A1>), native_func_((Store) native_func) {}

	template <typename A1, typename A2>
	NativeCall(Var(*native_func)(CallUnit* unit, A1*, A2*))
	:	call_(&func_map_seq<A1, A2>), native_func_((Store) native_func) {}

	template <typename A1, typename A2>
	void set(Var(*native_func)(A1*, A2*)) {
		call = &func_map_seq<A1, A2>;
		native_func_ = (void*) native_func;
	}

	Var call(CallUnit* unit, Seq args) {
		return call_(native_func_, unit, args);
	}

private:
	NativeMapper call_;
	void(*native_func_)(...);
	std::vector<String> names_;
};

JT_AST_NODE(NativeFuncCall) {
public:
	static const NodeType TYPE = NodeType::NATIVE_FUNC_CALL;

	NativeFuncCallImpl() : NodeImpl(this) {}

	NativeFuncCallImpl(NativeCall f)
	:	NodeImpl(this),
		native_(f) {}

	Seq args() { return args_; }
	void set_args(Seq args) { args_ = args; }

	Var do_call(CallUnit* unit, Seq args);

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	NativeCall native_;
	Seq args_;
};

class FuncTermImpl : public TermImpl {
public:
	static const TermType::Type TYPE = TermType::FUNC;

	FuncTermImpl() : TermImpl(this) {}

	Seq args() const { return args_; }
	void set_args(Seq a) { args_ = a; }

	Seq init_args() const { return init_args_; }
	void set_init_args(Seq a) { init_args_ = a; }

	Flow flow() const { return flow_; }
	void set_flow(Flow flow) { flow_ = flow; }

	Var ret() const { return ret_; }
	void set_ret(Var ret) { ret_ = ret; }

private:
	Seq args_;
	Flow flow_;
	Var ret_;

	Seq init_args_;
};

class AstPrinter : public IVisitor {
public:
	AstPrinter(std::ostream& out);

	virtual void caption(const char* title) override;
	virtual void caption(const char* title1, const char* title2) override;
	virtual void visit(const char* title, const Node& node) override;
	virtual void visit_term(const char* title, const Term& term) override;

protected:
	void print_offset(int local = 0);

protected:
	std::ostream& out_;

	int offset_;
};

class AstNodeCalls {
public:
	template <class N>
	void add(N node) {}

	template <class N>
	void call(N node) {}

private:
	//std::vector<>
};

class AstChecker : public IVisitor {
public:
	AstChecker();

	virtual void caption(const char* title) override;
	virtual void caption(const char* title1, const char* title2) override;
	virtual void visit(const char* title, const Node& node) override;
	virtual void visit_term(const char* title, const Term& term) override;

protected:
	AstNodeCalls checks_;
};

} // namespace jt {