#include "memory.hpp"
#include "logging.hpp"
#include <cassert>

using namespace moss;

#ifndef NDEBUG
long MemoryPool::allocated = 0;
#endif

void MemoryPool::store(opcode::Register reg, Value *v) {
    while (reg >= static_cast<opcode::Register>(pool.size())) {
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
    assert(v && "Loading non-existent value");
    return v;
}

void MemoryPool::store_name(opcode::Register reg, ustring name) {
    this->sym_table[name] = reg;
}

void MemoryPool::remove_name(ustring name) {
    auto pos = this->sym_table.find(name);
    assert(pos != sym_table.end() && "Name does not exist");
    this->sym_table.erase(pos);
}

Value *MemoryPool::load_name(ustring name, Interpreter *vm, Value **owner) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        auto v = this->pool[index->second];
        return v;
    }
    // Look for name also in spilled values
    for (auto riter = spilled_values.rbegin(); riter != spilled_values.rend(); ++riter) {
        // Skip anonymous spilled values accessed from other module
        if (auto spc = dyn_cast<SpaceValue>(*riter)) {
            if (spc->is_anonymous() && vm != spc->get_owner_vm()) {
                assert(spc->get_owner_vm() && "Anonymous space without owner");
                continue;
            }
        }
        auto val = (*riter)->get_attr(name, vm);
        if (val) {
            if (owner) {
                *owner = *riter;
            }
            return val;
        }
    }
    // Look for name in closures
    if (pool_fun_owner) {
        // Frames are pushed into closures from the back to front so no need for
        // reverse iteration
        for(auto f: pool_fun_owner->get_closures()) {
            auto val = f->load_name(name, vm, owner);
            if (val) {
                return val;
            }
        }

        if (pool_fun_owner->get_name() == name)
            return pool_fun_owner;
    }
    return nullptr;
}

std::optional<opcode::Register> MemoryPool::get_name_register(ustring name) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        return index->second;
    }
    return std::nullopt;
}

bool MemoryPool::overwrite(ustring name, Value *v, Interpreter *vm) {
    LOGMAX("Overwriting value " << name);
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        LOGMAX("Overwiting in pool");
        this->pool[index->second] = v;
        return true;
    }
    for (auto riter = spilled_values.rbegin(); riter != spilled_values.rend(); ++riter) {
        // Skip anonymous spilled values accessed from other module
        if (auto spc = dyn_cast<SpaceValue>(*riter)) {
            if (spc->is_anonymous() && vm != spc->get_owner_vm()) {
                assert(spc->get_owner_vm() && "Anonymous space without owner");
                continue;
            }
        }
        if ((*riter)->has_attr(name, vm)) {
            LOGMAX("Found as a spilled value of " << (*riter)->get_name());
            bool status = (*riter)->get_attrs()->overwrite(name, v, vm);
            assert(status && "Has attr but cannot be overwritten");
            return status;
        }
    }
    // Look for name in closures
    if (pool_fun_owner) {
        for(auto f: pool_fun_owner->get_closures()) {
            auto val = f->overwrite(name, v, vm);
            if (val) {
                return true;
            }
        }
    }
    return false;
}

void MemoryPool::push_finally(opcode::Finally *addr) {
    this->finally_stack.back().push_back(addr);
}

void MemoryPool::pop_finally() {
    assert(!this->finally_stack.empty() && "Trying to pop empty finally stack");
    assert(!this->finally_stack.back().empty() && "Trying to pop empty stack in finally stack");
    this->finally_stack.back().pop_back();
}

std::vector<opcode::Finally *> &MemoryPool::get_finally_stack() {
    return this->finally_stack.back();
}

void MemoryPool::push_finally_stack() {
    this->finally_stack.push_back({});
}

void MemoryPool::pop_finally_stack() {
    assert(!this->finally_stack.empty() && "Trying to pop stack from empty finally stack");
    this->finally_stack.pop_back();
}

size_t MemoryPool::get_finally_stack_size() {
    return this->finally_stack.size();
}

void MemoryPool::debug_sym_table(std::ostream& os, unsigned tab_depth) const {
    bool first = true;
    ++tab_depth;
    for (auto [k, v]: sym_table) {
        if (!v)
            continue;
        if (!first) {
            os << ",";
        }
        first = false;
        os << "\n";
        os << std::string(tab_depth*2, ' ') << "\"" << k << "\": " << *(this->pool[v]);
    }
    --tab_depth;
}

std::ostream& MemoryPool::debug(std::ostream& os) const {
    os << "> Symbol table:\n";
    for (auto [k, v] : this->sym_table) {
        os << "\"" << k << "\": " << v << " (" << *(this->pool[v]) << ")\n";
    }
    os << "> Memory pool:\n";
    size_t skip = 0;
    if (global) {
        skip = holds_consts ? BC_RESERVED_CREGS : 0;
        os << "-- Reserved regs (" << skip << ") skipped --\n";
    }
    for (size_t i = skip; i < this->pool.size(); ++i) {
        if (this->pool[i]) {
            os << i << ": " << *(this->pool[i]) << "\n";
        }
    }
    return os;
}