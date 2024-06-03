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
#include "os_interface.hpp"
#include <cstdint>
#include <list>

namespace moss {

class MemoryPool;
class Bytecode;

/**
 * @brief Interpreter for moss bytecode
 * Interpreter holds memory pools (stack frames) and runs bytecode provided
 */
class Interpreter {
private:
    Bytecode *code;

    MemoryPool *const_pool;
    std::list<MemoryPool *> reg_pools;

    opcode::Address bci;

    int exit_code;

    MemoryPool *get_const_pool() { return this->const_pool; }
    MemoryPool *get_reg_pool() { return this->reg_pools.back(); }
    MemoryPool *get_global_reg_pool() { return this->reg_pools.front(); }
public:
    Interpreter(Bytecode *code);
    ~Interpreter();

    void run();

    /** Stores a value into a register */
    void store(opcode::Register reg, Value *v);

    /** Stores a value into constant pool */
    void store_const(opcode::Register reg, Value *v);

    /** Sets a name for specific register */
    void store_name(opcode::Register reg, ustring name);

    /** 
     * Loads value at specified register index 
     * If there was no value stored, then Nil is stored there and returned
     */
    Value *load(opcode::Register reg);

    /** Loads a value from constant pool */
    Value *load_const(opcode::Register reg);

    /** 
     * Looks up a name and returns value corresponding to it in symbol table
     * If there is no such name, then exception is raised with name error 
     */
    Value *load_name(ustring name);

    /** 
     * Looks up a name in global frame and returns its value
     * If there is no such name, then exception is raised with name error 
     */
    Value *load_global_name(ustring name);

    // TODO: pop and push frame (with free)

    int get_exit_code() { return exit_code; }

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, Interpreter &i) {
    return i.debug(os);
}

}

#endif//_INTERPRETER_HPP_