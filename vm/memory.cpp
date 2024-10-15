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

/*void MemoryPool::store_fun_name(ustring name, opcode::Register reg) {
    this->sym_table[name] = reg;
    if (fun_names.find(name) != fun_names.end())
        fun_names[name].push_back(reg);
    else
        fun_names[name] = std::vector<ustring>{reg};
}*/

Value *MemoryPool::load_name(ustring name) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        auto v = this->pool[index->second];
        return v;
    }
    return nullptr;
}

/*std::pair<Value *, opcode::Register> MemoryPool::load_name_reg(ustring name) {
    auto index = this->sym_table.find(name);
    if (index != this->sym_table.end()) {
        auto v = this->pool[index->second];
        return std::make_pair(v, index->second);
    }
    return std::make_pair(nullptr, 0);
}*/

/*void MemoryPool::copy_names(opcode::Register from, opcode::Register to) {
    this->fun_names[to] = this->fun_names[from];
}*/

/*ustring MemoryPool::get_reg_pure_name(opcode::Register reg) {
    // TODO: Optimize
    auto names = get_reg_names(reg);
    if(names.empty())
        return "";
    return names[0];
}*/

/*std::vector<ustring> MemoryPool::get_reg_names(opcode::Register reg) {
    std::vector<ustring> names;
    names = reg_names[reg];
    // Push pure name as the first arg
    
    for (unsigned i = 1; i < names.size(); ++i) {
        if (names[i].find('(') == std::string::npos) {
            auto zero = names[0];
            names[0] = names[i];
            names[i] = zero;
            break;
        }
    }
    return names;
}*/

std::ostream& MemoryPool::debug(std::ostream& os) const {
    os << "> Symbol table:\n";
    for (auto [k, v] : this->sym_table) {
        os << "\"" << k << "\": " << v << " (" << *(this->pool[v]) << ")\n";
    }
    /*os << "> Reg names:\n";
    for (auto [k, v] : this->fun_names) {
        os << k << ": [";
        bool first = true;
        for (auto a: v) {
            if (first) {
                os << "\"" << a << "\"";
                first = false;
            } 
            else {
                os << ", \"" << a << "\""; 
            }
        }
        os << "] (" << *(this->pool[k]) << ")\n";
    }*/
    os << "> Memory pool:\n";
    for (size_t i = 0; i < this->pool.size(); ++i) {
        if (this->pool[i]) {
            os << i << ": " << *(this->pool[i]) << "\n";
        }
    }
    return os;
}