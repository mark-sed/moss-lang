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

namespace moss {

struct MemoryCell {
    Value *v;

    MemoryCell(Value *v) : v(v) {}
    MemoryCell() : v(nullptr) {}

    ~MemoryCell() {
        if (v) {
            delete v;
            v = nullptr;
        }
    }
};

class MemoryPool {
private:
    std::vector<MemoryCell> pool;
    std::map<ustring, register_t> sym_table;
public:
    MemoryPool() : pool(256) {}
    virtual ~MemoryPool() {}
};

}

#endif//_MEMORY_HPP_