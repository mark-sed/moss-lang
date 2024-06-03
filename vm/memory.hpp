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
    std::map<ustring, register_t> sym_table;
public:
    MemoryPool() : pool(256, nullptr) {}
    virtual ~MemoryPool() {
        for (auto v: pool) {
            if (v) {
                if (v->get_references() == 0)
                    delete v;
                v->dec_refs();
            }
        }
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
    /** 
     * Looks up a name and returns value corresponding to it in symbol table
     * If there is no such name, then exception is raised with name error 
     */
    Value *load_name(ustring name);

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, MemoryPool &pool) {
    return pool.debug(os);
}

}

#endif//_MEMORY_HPP_