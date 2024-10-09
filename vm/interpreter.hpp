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
#include "source.hpp"
#include "os_interface.hpp"
#include <cstdint>
#include <list>

namespace moss {

class MemoryPool;
class Bytecode;

/**
 * Frame that holds all call related information
 * The arguments, return register and argument type (const, addr, val)
 */
class CallFrame {
private:
    std::vector<Value *> args;
    opcode::Register return_reg;
    opcode::Address caller_addr;
public:
    CallFrame() : return_reg(0) {}

    void push_back(Value *v) { args.push_back(v); }
    void set_return_reg(opcode::Register r) { this->return_reg = r; }
    void set_caller_addr(opcode::Address addr) { this->caller_addr = addr; }
    
    std::vector<Value *> get_args() { return this->args; }
    opcode::Register get_return_reg() { return this->return_reg; }
    opcode::Address get_caller_addr() { return this->caller_addr; }
};

/**
 * @brief Interpreter for moss bytecode
 * Interpreter holds memory pools (stack frames) and runs bytecode provided
 */
class Interpreter {
private:
    Bytecode *code;
    File *src_file;
    MemoryPool *const_pool;
    std::list<MemoryPool *> frames;
    std::list<CallFrame *> call_frames;

    opcode::Address bci;

    int exit_code;

    bool bci_modified;

    MemoryPool *get_const_pool() { return this->const_pool; }
    MemoryPool *get_local_frame() { return this->frames.back(); }
    MemoryPool *get_global_frame() { return this->frames.front(); }
public:
    Interpreter(Bytecode *code, File *src_file=nullptr);
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

    ustring get_reg_name(opcode::Register reg);

    void push_frame();
    void pop_frame();

    CallFrame *get_call_frame() { 
        assert(!this->call_frames.empty() && "no call frame was pushed");
        return this->call_frames.back();
    }

    void push_call_frame() { call_frames.push_back(new CallFrame()); }
    void pop_call_frame() { 
        assert(!this->call_frames.empty() && "no call frame to pop");
        call_frames.pop_back(); 
    }

    int get_exit_code() { return exit_code; }

    std::ostream& debug(std::ostream& os) const;

    opcode::Address get_bci() { return this->bci; }
    void set_bci(opcode::Address v) { 
        this->bci = v;
        this->bci_modified = true; 
    }

    File *get_src_file() { return this->src_file; }
};

inline std::ostream& operator<< (std::ostream& os, Interpreter &i) {
    return i.debug(os);
}

}

#endif//_INTERPRETER_HPP_