
#include "python-bind.h"

#include <Python.h>

namespace jt {

PythonFuncCallImpl::PythonFuncCallImpl(const String& module_name, const String& func_name, Term ret_type)
:	NodeImpl(this),
	func_(nullptr),
	ret_(ret_type) {
	auto name = PyUnicode_FromString(strdup(module_name.c_str()));
	auto module = PyImport_Import(name);
	Py_DECREF(name);

	if (module == NULL) {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", module_name.c_str());
		//throw
	}

	auto func = PyObject_GetAttrString(module, strdup(func_name.c_str()));
	//Py_DECREF(module);

	if (!func || !PyCallable_Check(func)) {
		if (PyErr_Occurred())
			PyErr_Print();
		fprintf(stderr, "Cannot find function \"%s\"\n", func_name.c_str());
		//throw
	}
	else
		func_ = func;
}

PythonFuncCallImpl::~PythonFuncCallImpl() {
	Py_XDECREF(func_);
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
			fprintf(stderr, "Cannot convert argument\n");
			return make_ivar(-1);
		}
		PyTuple_SetItem(args, i, value);
	}

	auto func = (PyObject*)func_;
	PyObject* result = PyObject_CallObject((PyObject*)func_, args);
	Py_DECREF(args);
	if (result == NULL) {
		fprintf(stderr, "Call failed\n");
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
