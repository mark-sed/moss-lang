#include "memory.hpp"
#include "logging.hpp"
#include <cassert>

using namespace moss;

void MemoryPool::store(opcode::Register reg, Value *v) {
    if (reg >= static_cast<opcode::Register>(pool.size())) {
        LOGMAX("No more space, resizing pool from: " << pool.size() << " to " << pool.size()+(pool.size()/4));
        // TODO: Find some nice heuristic for this number
        pool.resize(pool.size()+(pool.size()/4), nullptr);
    }

    pool[reg] = v;
}

Value *MemoryPool::load(opcode::Register reg) {
    // FIXME
    assert(reg < static_cast<opcode::Register>(pool.size()) && "TODO: Pool access out of bounds, handle");
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

Value *MemoryPool::load_name(ustring name, Value **owner) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        auto v = this->pool[index->second];
        return v;
    }
    // Look for name also in spilled values
    for (auto riter = spilled_values.rbegin(); riter != spilled_values.rend(); ++riter) {
        auto val = (*riter)->get_attr(name);
        if (val) {
            if (owner) {
                *owner = *riter;
            }
            return val;
        }
    }
    return nullptr;
}

std::ostream& MemoryPool::debug(std::ostream& os) const {
    os << "> Symbol table:\n";
    for (auto [k, v] : this->sym_table) {
        os << "\"" << k << "\": " << v << " (" << *(this->pool[v]) << ")\n";
    }
    os << "> Memory pool:\n";
    size_t skip = 0;
    if (global) {
        skip = holds_consts ? BC_RESERVED_CREGS : BC_RESERVED_REGS;
        os << "-- Reserved regs (" << skip << ") skipped --\n";
    }
    for (size_t i = skip; i < this->pool.size(); ++i) {
        if (this->pool[i]) {
            os << i << ": " << *(this->pool[i]) << "\n";
        }
    }
    return os;
}