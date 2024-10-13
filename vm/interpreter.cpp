#include "interpreter.hpp"
#include "logging.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code, File *src_file) : code(code), src_file(src_file), bci(0), exit_code(0), bci_modified(false) {
    this->const_pool = new MemoryPool();
    // Global frame
    this->frames.push_back(new MemoryPool());
}

Interpreter::~Interpreter() {
    delete const_pool;
    for(auto p: frames) {
        delete p;
    }
    // Code is to be deleted by the creator of it
}

/*std::ostream& Interpreter::debug_pool(std::ostream &os) const {

}*/

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    unsigned index = 0;
    for (auto f: frames) {
        if (index == 0)
            os << "\t[" << index << "] Global frame:\n" << *f << "\n";
        else
            os << "\t[" << index << "] Local frame:\n" << *f << "\n";
        ++index;
    }
    os << "\tConstant pool:\n" << *const_pool << "\n";
    os << "\tExit code: " << exit_code << "\n";
    os << "}\n";
    return os;
}

void Interpreter::store(opcode::Register reg, Value *v) {
    get_local_frame()->store(reg, v);
}

void Interpreter::store_const(opcode::Register reg, Value *v) {
    get_const_pool()->store(reg, v);
}

Value *Interpreter::load(opcode::Register reg) {
    return get_local_frame()->load(reg);
}

Value *Interpreter::load_const(opcode::Register reg) {
    return get_const_pool()->load(reg);
}

void Interpreter::store_name(opcode::Register reg, ustring name) {
    get_local_frame()->store_name(reg, name);
}

Value *Interpreter::load_name(ustring name) {
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        auto val = (*riter)->load_name(name);
        if (val)
            return val;
    }
    return nullptr;
}

Value *Interpreter::load_global_name(ustring name) {
    return get_global_frame()->load_name(name);
}

ustring Interpreter::get_reg_pure_name(opcode::Register reg) {
    return get_local_frame()->get_reg_pure_name(reg);
}

std::vector<ustring> Interpreter::get_reg_names(opcode::Register reg) {
    return get_local_frame()->get_reg_names(reg);
}

std::pair<Value *, opcode::Register> Interpreter::load_name_reg(ustring name) {
    // TODO: Walk frames???
    return get_local_frame()->load_name_reg(name);
}

void Interpreter::copy_names(opcode::Register from, opcode::Register to) {
    // TODO: Walk frames??
    return get_local_frame()->copy_names(from, to);
}

void Interpreter::push_frame() {
    this->frames.push_back(new MemoryPool());
}

void Interpreter::pop_frame() {
    assert(frames.size() > 1 && "Trying to pop global frame");
    auto top = frames.back();
    frames.pop_back();
    delete top;
}

void Interpreter::run() {
    LOG1("Running interpreter\n----- OUTPUT: -----");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        opc->exec(this);

        // If bci was modified (jmp), then don't change it
        if (bci_modified)
            bci_modified = false;
        else
            ++bci;
    }

    LOG1("Finished interpreter");
}