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

/// \brief Executes a runtime function
/// \param vm VM for accessing resources
/// \param name Name of the function to execute
/// \param err Possible exception from execution
void dispatch(Interpreter *vm, ustring name, Value *&err);

/// \brief Creates mslib functions and pushes them into memory pool
/// \param gf Global frame
/// \param reg_counter Register number at which to start inserting functions
void init(MemoryPool *gf, opcode::Register &reg_counter);

}

}

#endif//_MSLIB_HPP_