#include "python.hpp"
#include "values_cpp.hpp"
#include "values.hpp"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <filesystem>

using namespace moss;
using namespace mslib;
using namespace python;

bool PYTHON_INITIALIZED = false;

static PyObject *moss2py(Interpreter *vm, CallFrame *cf, Value *v, Value *&err);
static Value *new_PythonObject(PyObject *ptr);

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
                auto nobj = new_PythonObject(pyv);
                if (err)
                    return nullptr;
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

static Value *new_PythonObject(PyObject *ptr) {
    auto pyocls = dyn_cast<ClassValue>(BuiltIns::PythonObject);
    auto obj = new ObjectValue(pyocls);
    // NOTE: If the name ptr is changed than it needs to be changed also in ~ObjectValue.
    obj->set_attr("ptr", new t_cpp::CVoidStarValue(ptr));
    return obj;
}

static PyObject *get_PyObject(Interpreter *vm, Value *v, Value *&err) {
    auto ptr_v = mslib::get_attr(v, "ptr", vm, err);
    if (err)
        return nullptr;
    auto cvoids = dyn_cast<t_cpp::CVoidStarValue>(ptr_v);
    assert(cvoids && "ptr is not void*");
    return static_cast<PyObject *>(cvoids->get_value());
}

Value *python::PythonObject(Interpreter *vm, CallFrame *cf, Value *, Value *ptr, Value *popul, Value *&err) {
    auto cvs = dyn_cast<t_cpp::CVoidStarValue>(ptr);
    assert(cvs);
    auto pyv = static_cast<PyObject *>(cvs->get_value());
    auto nobj = new_PythonObject(pyv);
    if (err)
        return nullptr;
    if (get_bool(cf->get_arg("populate"))) {
        python::populate(vm, cf, nobj, err);
    }
    return nobj;
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
    Value *ms_exc_obj = new_PythonObject(exc_obj);
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

    auto nobj = new_PythonObject(p_module);
    if (err)
        return nullptr;
    if (get_bool(popul)) {
        python::populate(vm, cf, nobj, err);
    }
    return nobj;
}

Value *python::PyObj_get(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *&err) {
    auto ptr = get_PyObject(vm, ths, err);
    if (err)
        return nullptr;
    auto name_str = mslib::get_string(name);
    PyObject *att = PyObject_GetAttrString(ptr, name_str.c_str());
    if (!att) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }
    return new_PythonObject(att);
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
    } else if (isa<ObjectValue>(v) && v->get_type() == BuiltIns::PythonObject) {
        return get_PyObject(vm, v, err);
    } else if (auto mv = dyn_cast<StringValue>(v)) {
        return PyUnicode_FromString(mv->get_value().c_str());
    } else if (isa<NilValue>(v)) {
        return Py_None;
    } else if (auto mv = dyn_cast<ListValue>(v)) {
        PyObject *lst = PyList_New(mv->size());
        size_t index = 0;
        for (auto elm: mv->get_vals()) {
            PyList_SetItem(lst, index, moss2py(vm, cf, elm, err));
            if (err)
                return nullptr;
            ++index;
        }
        return lst;
    } else if (auto mv = dyn_cast<DictValue>(v)) {
        PyObject *dct = PyDict_New();
        for (auto [k, v]: mv->vals_as_list()) {
            auto key = moss2py(vm, cf, k, err);
            if (err)
                return nullptr;
            auto val = moss2py(vm, cf, v, err);
            if (err)
                return nullptr;
            PyDict_SetItem(dct, key, val);
        }
        return dct;
    }
    // TODO: Add more types - dict, set
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
    } else if (t == &PyUnicode_Type) {
        return StringValue::get(PyUnicode_AsUTF8(obj));
    } else if (t == &PyList_Type) {
        Py_ssize_t size = PyList_Size(obj);
        auto lst = new ListValue();
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject *item = PyList_GetItem(obj, i);
            auto elm = py2moss(vm, cf, item, err);
            if (err) {
                return nullptr;
            }
            lst->push(elm);
        }
        return lst;
    } else if (t == &PyDict_Type) {
        Py_ssize_t pos = 0;
        PyObject *key, *value;
        std::vector<Value *> keys;
        std::vector<Value *> vals;
        while (PyDict_Next(obj, &pos, &key, &value)) {
            auto k = py2moss(vm, cf, key, err);
            if (err)
                return nullptr;
            auto d = py2moss(vm, cf, value, err);
            if (err)
                return nullptr;
            keys.push_back(k);
            vals.push_back(d);
        }
        auto dc = new DictValue();
        dc->push(keys, vals, vm);
        return dc;
    } else if (t == &PySet_Type) {
        PyObject *it = PyObject_GetIter(obj);
        PyObject *item;
        auto lst = new ListValue();
        while ((item = PyIter_Next(it))) {
            auto v = py2moss(vm, cf, item, err);
            if (err)
                return nullptr;
            lst->push(v);
        }
        return lst;
    }
    // TODO: Add more types - dict
    auto ce = create_PythonToMossConversionError(vm, cf, "No known conversion from Python type '"+ustring(t->tp_name)+"' to Moss type.", err);
    if (!err)
        err = ce;
    return nullptr;
}

Value *python::PyObj_call(Interpreter *vm, CallFrame *cf, Value *ths, Value *call_args, Value *&err) {
    auto ptr = get_PyObject(vm, ths, err);
    if (err)
        return nullptr;
    auto args = mslib::get_list(call_args);
    
    PyObject *py_args = PyTuple_New(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        PyTuple_SetItem(py_args, i, moss2py(vm, cf, args[i], err));
        if (err) {
            Py_DECREF(py_args);
            return nullptr;
        }
    }
    auto rval = PyObject_CallObject(ptr, py_args);
    Py_DECREF(py_args);
    if (!rval || PyErr_Occurred()) {
        auto exc = extract_py_exception(vm, cf, err);
        if (!err)
            err = exc; 
        return nullptr;
    }
    return new_PythonObject(rval);
}

Value *python::to_moss(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err) {
    auto ptr = get_PyObject(vm, ths, err);
    if (err)
        return nullptr;
    return py2moss(vm, cf, ptr, err);
}

Value *python::populate(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err) {
    auto ptr = get_PyObject(vm, ths, err);
    if (err)
        return nullptr;

    PyObject *dir_list = PyObject_Dir(ptr);
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
        PyObject *att = PyObject_GetAttrString(ptr, name);
        if (!att) {
            auto exc = extract_py_exception(vm, cf, err);
            if (!err)
                err = exc; 
            return nullptr;
        }
        ths->set_attr(name, new_PythonObject(att));
        if (err)
            return nullptr;
    }
    Py_DECREF(dir_list);
    return nullptr;
}

void python::init_constants(Interpreter *vm) {
    LOGMAX("Initialized Python");
#ifdef __windows__
#ifdef PYTHON_DLL_PATH
    std::filesystem::path dll_path = std::filesystem::path(PYTHON_DLL_PATH);
    std::filesystem::path python_root = dll_path.parent_path(); 
    LOGMAX("PYTHON_DLL_PATH set so setting home dir to: " << python_root.string());
    Py_SetPythonHome(python_root.wstring().c_str());
#endif
#endif
    Py_Initialize();
    PyRun_SimpleString("import sys; sys.path.append('.')");
    PYTHON_INITIALIZED = true;
}

void python::deinitialize_python() {
    if (PYTHON_INITIALIZED) {
        LOGMAX("Deinitializing Python");
        PYTHON_INITIALIZED = false;
        Py_Finalize();
    }
}
