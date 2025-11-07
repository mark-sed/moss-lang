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


const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

void init_constants(Interpreter *vm);
void deinitialize_python();

Value *module(Interpreter *vm, CallFrame *cf, Value *name, Value *popul, Value *&err);

Value *PythonObject(Interpreter *vm, CallFrame *cf, Value *, Value *ptr, Value *popul, Value *&err);

Value *PyObj_get(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *&err);

Value *PyObj_call(Interpreter *vm, CallFrame *cf, Value *ths, Value *args, Value *&err);

Value *to_moss(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err);

Value *populate(Interpreter *vm, CallFrame *cf, Value *ths, Value *&err);

}
}
}

#endif//_PYTHON_HPP_