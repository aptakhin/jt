
#include "python-bind.h"

#include <Python.h>

namespace jt {

int call_python() {

	std::vector<const char*> argv = { "", "builtins", "str", "2"};
	std::vector<String> types = {"str"};
	std::vector<String> res = {"str"};

	/*std::vector<const char*> argv = { "", "builtins", "int", "2"};
	std::vector<String> types = {"int"};
	std::vector<String> res = {"int"};*/
	int argc = argv.size();

	if (argc < 3) {
		fprintf(stderr, "Usage: call pythonfile funcname [args]\n");
		return 1;
	}

	Py_Initialize();
	auto name = PyUnicode_FromString(argv[1]);

	auto module = PyImport_Import(name);
	Py_DECREF(name);

	if (module == NULL) {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
		Py_Finalize();
		return 1;
	}

	auto func = PyObject_GetAttrString(module, argv[2]);

	if (!func || !PyCallable_Check(func)) {
		if (PyErr_Occurred())
			PyErr_Print();
		fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
	}

	auto args = PyTuple_New(argc - 3);
	for (int i = 0; i < argc - 3; ++i) {
		PyObject* value = nullptr;
		if (types[i] == "int")
			value = PyLong_FromLong(atoi(argv[i + 3]));
		else if (types[i] == "str")
			value = PyUnicode_FromString(argv[i + 3]);
		if (!value) {
			Py_DECREF(args);
			Py_DECREF(module);
			fprintf(stderr, "Cannot convert argument\n");
			return 1;
		}
		PyTuple_SetItem(args, i, value);
	}

	PyObject* result = PyObject_CallObject(func, args);
	Py_DECREF(args);
	if (result == NULL) {
		Py_DECREF(func);
		Py_DECREF(module);
		PyErr_Print();
		fprintf(stderr, "Call failed\n");
		return 1;
	}
	if (res[0] == "int")
		printf("Result of call: %ld\n", PyLong_AsLong(result));
	else if (res[0] == "str")
		printf("Result of call: %s\n", PyUnicode_AsUTF8(result));
				
	Py_DECREF(result);
	Py_XDECREF(func);
	Py_DECREF(module);

	Py_Finalize();
	return 0;
}

} // namespace jt {
