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
#include "os_interface.hpp"
#include "opcode.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <iostream>

namespace moss {

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
public:
    MemoryPool(bool holds_consts=false) : holds_consts(holds_consts) {
        if (holds_consts) {
            pool = std::vector<Value *>(BC_RESERVED_CREGS+256, nullptr);
        }
        else {
            pool = std::vector<Value *>(BC_RESERVED_REGS+256, nullptr);
        }
    }
    virtual ~MemoryPool() {
        // TODO:
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

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, MemoryPool &pool) {
    return pool.debug(os);
}

}

#endif//_MEMORY_HPP_