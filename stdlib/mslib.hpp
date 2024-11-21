/**
 * @file interpreter.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Moss bytecode interpreter
 * 
 * It connects all the VM parts into one and runs moss bytecode.
 */

#ifndef _MSLIB_HPP_
#define _MSLIB_HPP_

#include "interpreter.hpp"
#include "os_interface.hpp"
#include "memory.hpp"
#include <cassert>

namespace moss {

namespace mslib {

void exit(Interpreter *vm, Value *code);
void vardump(Interpreter *vm, Value *v);

void dispatch(Interpreter *vm, ustring name, Value *&err);

void init(MemoryPool *gf, opcode::Register &reg_counter);

}

}

#endif//_MSLIB_HPP_