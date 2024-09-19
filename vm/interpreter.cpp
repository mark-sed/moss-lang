#include "interpreter.hpp"
#include "logging.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code, File *src_file) : code(code), src_file(src_file), bci(0), exit_code(0) {
    this->const_pool = new MemoryPool();
    // Global frame
    this->frames.push_back(new MemoryPool());
}

Interpreter::~Interpreter() {
    delete const_pool;
    for(auto p: frames) {
        delete p;
    }
}

/*std::ostream& Interpreter::debug_pool(std::ostream &os) const {

}*/

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    // TODO: Debug all pools
    os << "\tRegister pool:\n" << *frames.back() << "\n";
    os << "\tConstant pool:\n" << *const_pool << "\n";
    os << "\tExit code: " << exit_code << "\n";
    os << "}\n";
    return os;
}

void Interpreter::store(opcode::Register reg, Value *v) {
    // TODO: Go through frames
    // FIXME: Incorrect
    get_local_frame()->store(reg, v);
}

void Interpreter::store_const(opcode::Register reg, Value *v) {
    // TODO: Go through frames
    // FIXME: Incorrect
    get_const_pool()->store(reg, v);
}

Value *Interpreter::load(opcode::Register reg) {
    // TODO: Go through frames
    // FIXME: Incorrect
    return get_local_frame()->load(reg);
}

Value *Interpreter::load_const(opcode::Register reg) {
    // TODO: Go through frames
    // FIXME: Incorrect
    return get_const_pool()->load(reg);
}

void Interpreter::store_name(opcode::Register reg, ustring name) {
    // TODO: Go through frames
    // FIXME: Incorrect
    get_local_frame()->store_name(reg, name);
}

Value *Interpreter::load_name(ustring name) {
    // TODO: Go through frames
    // FIXME: Incorrect
    return get_local_frame()->load_name(name);
}

Value *Interpreter::load_global_name(ustring name) {
    return get_global_frame()->load_name(name);
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

    // TODO: Change for repl
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