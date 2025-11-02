/// 
/// \file python.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///
/// This contains internal implementations of sys.ms module
/// 

#ifndef _PYTHON_HPP_
#define _PYTHON_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"
#include "values_cpp.hpp"
#include <Python.h>

namespace moss {
namespace mslib {

/// This namespace hold methods of python module in mslib.
namespace python {

/// Moss version of PyObject, holds the pointer to it.
/// This is needed to decrement the reference counter on value deletion.
class PythonObjectValue : public Value {
private:
    PyObject *ptr;
public:
    static const TypeKind ClassType = TypeKind::PYTHON_OBJ;

    PythonObjectValue(PyObject *ptr);
    ~PythonObjectValue();

    Value *clone() override {
        Py_XINCREF(ptr);
        return new PythonObjectValue(ptr);
    }

    PyObject *get_value() {
        return this->ptr;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<size_t>{}((size_t)ptr);
    }

    virtual opcode::StringConst as_string() const override {
        return "<object of class PythonObject>";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "PythonObject(" << ptr << ")";
        return os;
    }
};

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

void init_constants(Interpreter *vm);

Value *module(Interpreter *vm, CallFrame *cf, Value *name, Value *&err);

Value *PythonObject(Interpreter *vm, Value *ths, Value *ptr, Value *&err);

Value *PyObj_get(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *&err);

Value *PyObj_call(Interpreter *vm, CallFrame *cf, Value *ths, Value *args, Value *&err);

Value *to_moss(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err);

}
}
}

#endif//_PYTHON_HPP_