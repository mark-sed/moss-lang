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
#include <vector>

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
    std::vector<MemoryCell *> pool; 
public:
    MemoryPool() : pool(256) {}
    virtual ~MemoryPool() {
        for (auto *c: pool) {
            delete c;
        }
    }
};

}

#endif//_MEMORY_HPP_