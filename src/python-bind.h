// jt
//
#pragma once

#include "common.h"
#include "ast.h"

namespace jt {

JT_AST_NODE(PythonFuncCall) {
public:
	static const NodeType TYPE = NodeType::PYTHON_FUNC_CALL;

	PythonFuncCallImpl() : NodeImpl(this) {}

	PythonFuncCallImpl(const String& module, const String& name, Term ret_type);

	~PythonFuncCallImpl();

	virtual PythonFuncCallImpl* do_clone() const override;

	Seq args() const { return args_; }
	void set_args(Seq args) { args_ = args; }

	Var do_call(CallUnit* unit, FuncTermImpl* parent, Seq args);

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	Seq args_;
	void* func_;
	Term ret_;
};

class PyEnv {
public:
	PyEnv();
	~PyEnv();
};

} // namespace jt {

