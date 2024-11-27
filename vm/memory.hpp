/**
 * @file memory.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Memory pools for the vm
 */

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

/**
 * @brief Virtual memory representation
 * It holds pool of values with reference counting for their garbage collection
 * and it also holds symbol table which has the variable names and corresponding
 * index into the pool.
 */
class MemoryPool {
private:
    std::vector<Value *> pool;
    std::map<ustring, opcode::Register> sym_table;
    bool holds_consts;
    bool global;
public:
    MemoryPool(bool holds_consts=false, bool global=false) : holds_consts(holds_consts), global(global) {
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
    }
    virtual ~MemoryPool() {
        // Values are deleted by gc
    }

    /** Stores a value into a register */
    void store(opcode::Register reg, Value *v);
    /** 
     * Loads value at specified register index 
     * If there was no value stored, then Nil is stored there and returned
     */
    Value *load(opcode::Register reg);

    /** Sets a name for specific register */
    void store_name(opcode::Register reg, ustring name);

    Value *load_name(ustring name);

    size_t get_free_reg() {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!pool[i]) return i;
        }
        pool.push_back(nullptr);
        return pool.size()-1;
    }

    std::vector<Value *> &get_pool() { return this->pool; }

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, MemoryPool &pool) {
    return pool.debug(os);
}

}

#endif//_MEMORY_HPP_