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
#include <cstdint>
#include <list>

namespace moss {

class MemoryPool;
class Bytecode;

class Interpreter {
private:
    Bytecode *code;

    MemoryPool *const_pool;
    std::list<MemoryPool *> reg_pools;

    uint32_t bci;

    int exit_code;
public:
    Interpreter(Bytecode *code);
    ~Interpreter();

    void run();

    MemoryPool *get_const_pool() { return this->const_pool; }
    MemoryPool *get_reg_pool() { return this->reg_pools.back(); }
    MemoryPool *get_global_reg_pool() { return this->reg_pools.front(); }

    // TODO: pop and push frame (with free)

    int get_exit_code() { return exit_code; }

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, Interpreter &i) {
    return i.debug(os);
}

}

#endif//_INTERPRETER_HPP_