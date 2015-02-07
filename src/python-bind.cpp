
#include "python-bind.h"

#include <Python.h>

namespace jt {

PythonFuncCallImpl::PythonFuncCallImpl(String module_name, String func_name, Term ret_type)
:	NodeImpl(this),
	module_name_(std::move(module_name)),
	func_name_(std::move(func_name)),
	ret_(ret_type),
	py_func_(nullptr, [] (void* func) { Py_XDECREF(func); } ),
	py_module_(nullptr, [] (void* module) { Py_DECREF(module); } ) {
	auto name = PyUnicode_FromString(module_name_.c_str());
	py_module_.reset(PyImport_Import(name));
	Py_DECREF(name);

	if (py_module_.get() == nullptr) {
		PyErr_Print();
		JT_TR(String() + "Failed to load:" + module_name_, PYTHON_NOTIF);
	}

	auto func = PyObject_GetAttrString((PyObject*) py_module_.get(), func_name_.c_str());

	if (!func || !PyCallable_Check(func)) {
		if (PyErr_Occurred())
			PyErr_Print();
		JT_TR(String() + "Cannot find function: " + func_name_ + " in module: " + module_name_, PYTHON_NOTIF);
		//throw
	}
	else
		py_func_.reset(func);
}

PythonFuncCallImpl* PythonFuncCallImpl::do_clone() const {
	auto func = new PythonFuncCallImpl{};
	func->set_args(Seq(args()->do_clone()));
	return func;
}

Var PythonFuncCallImpl::do_call(CallUnit* unit, FuncTermImpl* parent, Seq set_args) {
	auto size = set_args->vars().size();
	auto args = PyTuple_New(size);
	for (int i = 0; i < size; ++i) {
		PyObject* value = nullptr;
		if (auto integer = set_args->vars()[i]->term().as<IntTermImpl>())
			value = PyLong_FromLong(integer->number());
		else if (auto str = set_args->vars()[i]->term().as<StringTermImpl>())
			value = PyUnicode_FromString(str->str().c_str());
		if (!value) {
			Py_DECREF(args);
			JT_TR("Cannot convert argument", PYTHON_NOTIF);
			return make_ivar(-1);
		}
		PyTuple_SetItem(args, i, value);
	}

	PyObject* result = PyObject_CallObject((PyObject*) py_func_.get(), args);
	Py_DECREF(args);
	if (result == NULL) {
		JT_TR("Call failed", PYTHON_NOTIF);
		return make_ivar(-1);
	}

	if (auto integer = ret_.as<IntTermImpl>())
		return make_ivar(PyLong_AsLong(result));
	else if (auto str = ret_.as<StringTermImpl>()) {
		auto x = PyUnicode_AsUTF8(result);
		return make_svar(x);
	}

	return make_ivar(-1);
}

void PythonFuncCallImpl::do_visit(IVisitor* vis) const {
	vis->caption("PythonFuncCall");
}

PyEnv::PyEnv() {
	Py_Initialize();
}

PyEnv::~PyEnv() {
	Py_Finalize();
}

} // namespace jt {
