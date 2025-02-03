/// 
/// \file interpreter.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss bytecode interpreter
/// 
/// It connects all the VM parts into one and runs moss bytecode.
/// 

#ifndef _MSLIB_HPP_
#define _MSLIB_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "memory.hpp"
#include "values.hpp"
#include <cassert>

namespace moss {

namespace mslib {

void exit(Interpreter *vm, Value *code);
Value *vardump(Interpreter *vm, Value *v);
Value *print(Interpreter *vm, Value *msgs, Value *end, Value *separator);

Value *Int(Interpreter *vm, Value *ths, Value *v, Value *base);
Value *Exception(Interpreter *vm, Value *ths, Value *msg);

/// \brief Executes a runtime function
/// \param vm VM for accessing resources
/// \param name Name of the function to execute
/// \param err Possible exception from execution
void dispatch(Interpreter *vm, ustring name, Value *&err);

Value *create_name_error(ustring msg);

}

}

#endif//_MSLIB_HPP_