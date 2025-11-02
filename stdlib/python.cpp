#include "python.hpp"
#include "values_cpp.hpp"
#define PY_SSIZE_T_CLEAN
#include <Python.h>

using namespace moss;
using namespace mslib;
using namespace python;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& python::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"call", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            auto ths = cf->get_arg("this");
            auto val = cf->get_arg("args");
            return python::PyObj_call(vm, cf, ths, val, err);
        }},
        {"get", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            return python::PyObj_get(vm, cf, cf->get_arg("this"), cf->get_arg("name"), err);
        }},
        {"module", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return python::module(vm, cf, args[0].value, err);
        }},
        {"PythonObject", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            return python::PythonObject(vm, cf->get_arg("this"), cf->get_arg("ptr"), err);
        }},
        
    };
    return registry;
}

PythonObjectValue::PythonObjectValue(PyObject *ptr) 
    : Value(ClassType, "<object of PythonObject>", BuiltIns::PythonObject), ptr(ptr) {
    if(BuiltIns::PythonObject->get_attrs())
        this->attrs = BuiltIns::PythonObject->get_attrs()->clone();
}

PythonObjectValue::~PythonObjectValue() {
    Py_XDECREF(ptr);
}

static ustring get_py_exception(PyObject **exc_out) {
    PyObject *type = nullptr, *value = nullptr, *traceback = nullptr;

    // Fetch and normalize the current Python exception
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    if (exc_out) {
        // Return the actual exception object (e.g. ImportError instance)
        *exc_out = value ? value : Py_None;
        Py_XINCREF(*exc_out);
    }
    // Convert the exception object to a string
    PyObject *msg = NULL;
    if (value) {
        msg = PyObject_Str(value); // calls value.__str__()
    } else if (type) {
        msg = PyObject_Str(type);
    } else {
        msg = PyUnicode_FromString("Unknown Python error");
    }

    ustring str_msg = PyUnicode_AsUTF8(msg);
    Py_XDECREF(msg);

    Py_XDECREF(type);
    Py_XDECREF(traceback);

    return str_msg;
}

static Value *extract_py_exception(Interpreter *vm, CallFrame *cf, Value *&err) {
    PyObject *exc_obj = nullptr;
    auto msg = get_py_exception(&exc_obj);
    Value *ms_exc_obj = new PythonObjectValue(exc_obj);
    if (err)
        return nullptr;
    return mslib::call_constructor(vm, cf, "PythonException", {StringValue::get(msg), ms_exc_obj}, err);
}

Value *python::module(Interpreter *vm, CallFrame *cf, Value *name, Value *&err) {
    auto name_str = mslib::get_string(name);
    PyObject *p_name = PyUnicode_DecodeFSDefault(name_str.c_str());
    assert(p_name);
    PyObject *p_module = PyImport_Import(p_name);
    Py_DECREF(p_name);

    if (p_module == nullptr) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }

    return new PythonObjectValue(p_module);
}

Value *python::PyObj_get(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *&err) {
    PythonObjectValue *po = dyn_cast<PythonObjectValue>(ths);
    assert(po && "This is not PythonObject");
    auto name_str = mslib::get_string(name);
    PyObject *att = PyObject_GetAttrString(po->get_value(), name_str.c_str());
    if (!att) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }
    return new PythonObjectValue(att);
}

Value *python::PyObj_call(Interpreter *vm, CallFrame *cf, Value *ths, Value *call_args, Value *&err) {
    PythonObjectValue *po = dyn_cast<PythonObjectValue>(ths);
    assert(po && "This is not PythonObject");
    auto args = mslib::get_list(call_args);
    // TODO
    return nullptr;
}

Value *python::PythonObject(Interpreter *vm, Value *ths, Value *ptr, Value *&err) {
    auto cvs = dyn_cast<t_cpp::CVoidStarValue>(ptr);
    assert(cvs);
    return new PythonObjectValue(static_cast<PyObject *>(cvs->get_value()));
}

void python::init_constants(Interpreter *vm) {
    LOGMAX("Initialized Python");
    Py_Initialize();
    // FIXME: Change the path to be moss path
    PyRun_SimpleString("import sys; sys.path.append('.')");
}

template<>
mslib::python::PythonObjectValue *moss::dyn_cast(Value* t) {
    assert(t && "Passed nullptr to dyn_cast");
    if (t->get_kind() == TypeKind::PYTHON_OBJ)
        return dynamic_cast<mslib::python::PythonObjectValue *>(t);
    return nullptr;
}