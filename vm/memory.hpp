/// 
/// \file memory.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Memory pools for the vm
/// 

#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include "values.hpp"
#include "commons.hpp"
#include "opcode.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <iostream>

namespace moss {

class Value;
class FunValue;

namespace opcode {
    class Finally;
}

/// \brief Virtual memory representation
/// It holds pool of values with reference counting for their garbage collection
/// and it also holds symbol table which has the variable names and corresponding
/// index into the pool. 
class MemoryPool {
private:
    FunValue *pool_fun_owner; ///< This value is set to the owner of this pool if it is a function
    std::vector<Value *> pool;
    std::map<ustring, opcode::Register> sym_table;
    std::list<Value *> spilled_values;   ///< Modules and spaces imported and spilled into global scope
    std::vector<std::vector<opcode::Finally *>> finally_stack;

    bool holds_consts;
    bool global;
    bool marked;
public:
#ifndef NDEBUG
    static long allocated;
#endif
    MemoryPool(bool holds_consts=false, bool global=false) : pool_fun_owner(nullptr), holds_consts(holds_consts), global(global), marked(false) {
        if (!global && !holds_consts) {
            // TODO: Fine tune these values
            pool = std::vector<Value *>(128, nullptr);
        }
        else if (holds_consts) {
            pool = std::vector<Value *>(BC_RESERVED_CREGS+256, nullptr);
        }
        else {
            pool = std::vector<Value *>(BC_RESERVED_REGS+256, nullptr);
        }
        this->finally_stack.push_back({});
#ifndef NDEBUG
        ++allocated;
#endif
    }
    MemoryPool *clone() {
        auto cpy = new MemoryPool(holds_consts, global);
        cpy->pool = pool;
        cpy->sym_table = sym_table;
        cpy->spilled_values = spilled_values;
        return cpy;
    }
    ~MemoryPool() {
        // Values are deleted by gc
#ifndef NDEBUG
        --allocated;
#endif
    }

    /// Stores a value into a register
    void store(opcode::Register reg, Value *v); 
    /// Loads value at specified register index 
    /// If there was no value stored, then Nil is stored there and returned
    Value *load(opcode::Register reg);

    /// Sets a name for specific register
    void store_name(opcode::Register reg, ustring name);

    /// Removes a name from symbol table.
    void remove_name(ustring name);

    /// Loads a value based on a name.
    /// \param name Name to look up.
    /// \param vm Interpreter which does this call. This is used to check if
    ///           values from anonymous namespace should be returned.
    /// \param owner Returns value which owns this value if the value is in a
    ///              closure.
    Value *load_name(ustring name, Interpreter *vm, Value **owner=nullptr);

    bool overwrite(ustring name, Value *v, Interpreter *vm);

    /// Spills a new value.
    void push_spilled_value(Value *v) {
        this->spilled_values.push_back(v);
    }

    /// \return first free register
    size_t get_free_reg() {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!pool[i]) return i;
        }
        pool.push_back(nullptr);
        return pool.size()-1;
    }

    std::vector<Value *> &get_pool() { return this->pool; }
    std::list<Value *> &get_spilled_values() { return this->spilled_values; }

    /// \return true if frame is global frame
    bool is_global() { return this->global; }

    bool is_empty_sym_table() { return this->sym_table.empty(); }

    /// Sets owner of this pool to a function if this is a closure frame
    void set_pool_fun_owner(FunValue *f) { this->pool_fun_owner = f; }

    void set_marked(bool m) { this->marked = m; }
    bool is_marked() { return this->marked; }

    void push_finally(opcode::Finally *addr);
    void pop_finally();
    void push_finally_stack();
    void pop_finally_stack();

    std::vector<opcode::Finally *> &get_finally_stack();
    size_t get_finally_stack_size();

    std::ostream& debug(std::ostream& os) const;
    void debug_sym_table(std::ostream& os, unsigned tab_depth=0) const;

    /// \return list of all the keys in the symbol table
    std::list<ustring> get_sym_table_keys() {
        std::list<ustring> keys;
        for (auto &[k, _]: sym_table) {
            keys.push_back(k);
        }
        return keys;
    }
};

inline std::ostream& operator<< (std::ostream& os, MemoryPool &pool) {
    return pool.debug(os);
}

}

#endif//_MEMORY_HPP_