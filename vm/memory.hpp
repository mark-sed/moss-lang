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

/*struct MemoryCell {
    Value *v;

    MemoryCell(Value *v) : v(v) {}
    MemoryCell() : v(nullptr) {}

    ~MemoryCell() {
        if (v) {
            delete v;
            v = nullptr;
        }
    }
};*/

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

    void store(register_t reg, Value *v);
    Value *load(register_t reg);

    void store_name(register_t reg, ustring name);
    Value *load_name(ustring name);

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, MemoryPool &pool) {
    return pool.debug(os);
}

}

#endif//_MEMORY_HPP_