#include "memory.hpp"
#include <cassert>

using namespace moss;

void MemoryPool::store(register_t reg, Value *v) {
    assert(reg > static_cast<register_t>(pool.size()) && "TODO: Resize pool when out of bounds");
    if (pool[reg]) {
        delete pool[reg];
    }
    pool[reg] = v;
}

std::ostream& MemoryPool::debug(std::ostream& os) const {
    for (auto [k, v] : this->sym_table) {
        os << k << ": " << v << "\n";
    }
    for (size_t i = 0; i < this->pool.size(); ++i) {
        if (this->pool[i]) {
            os << i << ": " << *(this->pool[i]) << "\n";
        }
    }
    return os;
}