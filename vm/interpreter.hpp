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

#ifndef _INTERPRETER_HPP_
#define _INTERPRETER_HPP_

#include "memory.hpp"
#include "bytecode.hpp"

namespace moss {

class Interpreter {
private:
    Bytecode *code;

    MemoryPool *const_pool;
    MemoryPool *reg_pool;

public:
    Interpreter(Bytecode *code) : code(code) {
        this->const_pool = new MemoryPool();
        this->reg_pool = new MemoryPool();
    }

    ~Interpreter() {
        delete const_pool;
        delete reg_pool;
    }

    void run();
};

}

#endif//_INTERPRETER_HPP_