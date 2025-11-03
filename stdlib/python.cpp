#include "python.hpp"
#include "values_cpp.hpp"
#define PY_SSIZE_T_CLEAN
#include <Python.h>

using namespace moss;
using namespace mslib;
using namespace python;

static PyObject *moss2py(Interpreter *vm, CallFrame *cf, Value *v, Value *&err);

const std::unordered_map<std::string, mslib::mslib_dispatcher>& python::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"call", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            auto ths = cf->get_arg("this");
            auto val = cf->get_arg("args");
            return python::PyObj_call(vm, cf, ths, val, err);
        }},
        {"get", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return python::PyObj_get(vm, cf, cf->get_arg("this"), cf->get_arg("name"), err);
        }},
        {"module", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return python::module(vm, cf, cf->get_arg("name"), cf->get_arg("populate"), err);
        }},
        {"populate", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 1);
            return python::populate(vm, cf, args[0].value, err);
        }},
        {"PythonObject", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 3);
            if (cf->get_arg("ptr")) {
                return python::PythonObject(vm, cf, cf->get_arg("this"), cf->get_arg("ptr"), cf->get_arg("populate"), err);
            } else {
                assert(cf->get_arg("v"));
                auto pyv = moss2py(vm, cf, cf->get_arg("v"), err);
                if (err)
                    return nullptr;
                auto nobj = new PythonObjectValue(pyv);
                if (get_bool(cf->get_arg("populate"))) {
                    python::populate(vm, cf, nobj, err);
                }
                return nobj;
            }
        }},
        {"to_moss", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 1);
            return python::to_moss(vm, cf, args[0].value, err);
        }},
        
    };
    return registry;
}

PythonObjectValue::PythonObjectValue(PyObject *ptr) 
    : ObjectValue(dyn_cast<ClassValue>(BuiltIns::PythonObject)), ptr(ptr) {
    this->kind = PythonObjectValue::ClassType;
    this->name = "<python object>";
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
    return mslib::call_constructor(vm, cf, "PythonException", {StringValue::get(msg), ms_exc_obj}, err);
}

static Value *create_PythonToMossConversionError(Interpreter *vm, CallFrame *cf, ustring msg, Value *&err) {
    return mslib::call_constructor(vm, cf, "PythonToMossConversionError", {StringValue::get(msg)}, err);
}

static Value *create_MossToPythonConversionError(Interpreter *vm, CallFrame *cf, ustring msg, Value *&err) {
    return mslib::call_constructor(vm, cf, "MossToPythonConversionError", {StringValue::get(msg)}, err);
}

Value *python::module(Interpreter *vm, CallFrame *cf, Value *name, Value *popul, Value *&err) {
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

    auto nobj = new PythonObjectValue(p_module);
    if (get_bool(popul)) {
        python::populate(vm, cf, nobj, err);
    }
    return nobj;
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

static PyObject *moss2py(Interpreter *vm, CallFrame *cf, Value *v, Value *&err) {
    if (auto mv = dyn_cast<IntValue>(v)) {
        return PyLong_FromLongLong(mv->get_value());
    } else if (auto mv = dyn_cast<FloatValue>(v)) {
        return PyFloat_FromDouble(mv->get_value());
    } else if (auto mv = dyn_cast<BoolValue>(v)) {
        if (mv->get_value())
            Py_RETURN_TRUE;
        Py_RETURN_FALSE;
    } else if (auto mv = dyn_cast<PythonObjectValue>(v)) {
        return mv->get_value();
    }
    // TODO: Add more types
    auto ce = create_MossToPythonConversionError(vm, cf, "No known conversion from Moss type '"+v->get_type()->get_name()+"' to Python type.", err);
    if (!err)
        err = ce;
    return nullptr;
}

static Value *py2moss(Interpreter *vm, CallFrame *cf, PyObject *obj, Value *&err) {
    PyTypeObject *t = Py_TYPE(obj);
    if (t == &PyLong_Type) {
        return IntValue::get(PyLong_AsLongLong(obj));
    } else if (t == &PyBool_Type) {
        return BoolValue::get(static_cast<bool>(PyLong_AsLong(obj)));
    } else if (t == &PyFloat_Type) {
        return FloatValue::get(PyFloat_AsDouble(obj));
    } else if (obj == Py_None) {
        return NilValue::Nil();
    }
    // TODO: Add more types
    auto ce = create_PythonToMossConversionError(vm, cf, "No known conversion from Python type '"+ustring(t->tp_name)+"' to Moss type.", err);
    if (!err)
        err = ce;
    return nullptr;
}

Value *python::PyObj_call(Interpreter *vm, CallFrame *cf, Value *ths, Value *call_args, Value *&err) {
    PythonObjectValue *po = dyn_cast<PythonObjectValue>(ths);
    assert(po && "This is not PythonObject");
    auto args = mslib::get_list(call_args);
    
    PyObject *py_args = PyTuple_New(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        PyTuple_SetItem(py_args, i, moss2py(vm, cf, args[i], err));
        if (err) {
            Py_DECREF(py_args);
            return nullptr;
        }
    }
    auto rval = PyObject_CallObject(po->get_value(), py_args);
    Py_DECREF(py_args);
    if (!rval || PyErr_Occurred()) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }
    return new PythonObjectValue(rval);
}

Value *python::to_moss(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err) {
    PythonObjectValue *po = dyn_cast<PythonObjectValue>(ths);
    assert(po && "This is not PythonObject");
    return py2moss(vm, cf, po->get_value(), err);
}

Value *python::populate(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err) {
    PythonObjectValue *po = dyn_cast<PythonObjectValue>(ths);
    assert(po && "This is not PythonObject");

    PyObject *dir_list = PyObject_Dir(po->get_value());
    if (!dir_list) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }
    Py_ssize_t len = PyList_Size(dir_list);
    for (Py_ssize_t i = 0; i < len; ++i) {
        PyObject *item = PyList_GetItem(dir_list, i);  // Borrowed reference
        const char *name = PyUnicode_AsUTF8(item);
        PyObject *att = PyObject_GetAttrString(po->get_value(), name);
        if (!att) {
            auto exc = extract_py_exception(vm, cf, err);
            if (!err)
                err = exc; 
            return nullptr;
        }
        ths->set_attr(name, new PythonObjectValue(att));
    }
    Py_DECREF(dir_list);
    return nullptr;
}

Value *python::PythonObject(Interpreter *vm, CallFrame *cf, Value *, Value *ptr, Value *popul, Value *&err) {
    auto cvs = dyn_cast<t_cpp::CVoidStarValue>(ptr);
    assert(cvs);
    auto nobj = new PythonObjectValue(static_cast<PyObject *>(cvs->get_value()));
    if (get_bool(popul)) {
        python::populate(vm, cf, nobj, err);
    }
    return nobj;
}

void python::init_constants(Interpreter *vm) {
    LOGMAX("Initialized Python");
    Py_Initialize();
    // FIXME: Change the path to be moss path
    PyRun_SimpleString("import sys; sys.path.append('.')");
}

std::ostream& PythonObjectValue::debug(std::ostream& os) const {
    // TODO: Output all needed debug info
    os << "Object : " << type->get_name() << " {"; 
    if (!attrs || attrs->is_empty_sym_table()) {
        os << "}";
    }
    else {
        attrs->debug_sym_table(os, tab_depth);
        os << "\n" << std::string(tab_depth*2, ' ') << "}";
    }

    return os;
}

template<>
mslib::python::PythonObjectValue *moss::dyn_cast(Value* t) {
    assert(t && "Passed nullptr to dyn_cast");
    if (t->get_kind() == TypeKind::PYTHON_OBJ)
        return dynamic_cast<mslib::python::PythonObjectValue *>(t);
    return nullptr;
}