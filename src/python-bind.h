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

	PythonFuncCallImpl(String module_name, String func_name, Term ret_type);

	virtual PythonFuncCallImpl* do_clone() const override;

	Seq args() const { return args_; }
	void set_args(Seq args) { args_ = args; }

	Var do_call(CallUnit* unit, FuncTermImpl* parent, Seq args);

	virtual void do_visit(IVisitor* visitor) const override;

protected:
	String module_name_;
	String func_name_;
	Seq args_;
	cres_ptr<void> py_func_;
	cres_ptr<void> py_module_;
	Term ret_;
};

class PyEnv {
public:
	PyEnv();
	~PyEnv();
};

} // namespace jt {

