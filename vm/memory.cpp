#include "memory.hpp"
#include "logging.hpp"
#include <cassert>

using namespace moss;

void MemoryPool::store(opcode::Register reg, Value *v) {
    // FIXME
    assert(reg < static_cast<opcode::Register>(pool.size()) && "TODO: Resize pool when out of bounds");
    if (pool[reg]) {
        assert(pool[reg]->get_references() > 0 && "TODO: Collect value that would be overriden and lost");
    }
    pool[reg] = v;
}

Value *MemoryPool::load(opcode::Register reg) {
    // FIXME
    assert(reg < static_cast<opcode::Register>(pool.size()) && "TODO: Resize pool when out of bounds");
    Value *v = pool[reg];
    if (v)
        return v;
    // FIXME: return nil
    assert(false && "TODO: Nil return for no value");
    return nullptr;
}

void MemoryPool::store_name(opcode::Register reg, ustring name) {
    this->sym_table[name] = reg;
}

Value *MemoryPool::load_name(ustring name) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        auto v = this->pool[index->second];
        return v;
    }
    return nullptr;
}

ustring MemoryPool::get_reg_name(opcode::Register reg) {
    for (auto [k, v]: sym_table) {
        // Return only pure name, not function name
        if (v == reg && k.find('(') == std::string::npos)
            return k;
    }
    return "";
}

std::ostream& MemoryPool::debug(std::ostream& os) const {
    os << "> Symbol table:\n";
    for (auto [k, v] : this->sym_table) {
        os << "\"" << k << "\": " << v << " (" << *(this->pool[v]) << ")\n";
    }
    os << "> Memory pool:\n";
    for (size_t i = 0; i < this->pool.size(); ++i) {
        if (this->pool[i]) {
            os << i << ": " << *(this->pool[i]) << "\n";
        }
    }
    return os;
}