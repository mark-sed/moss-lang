#include "interpreter.hpp"
#include "logging.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code) : code(code), bci(0), exit_code(0) {
    this->const_pool = new MemoryPool();
    this->reg_pools.push_back(new MemoryPool());
}

Interpreter::~Interpreter() {
    delete const_pool;
    for(auto p: reg_pools) {
        delete p;
    }
}

/*std::ostream& Interpreter::debug_pool(std::ostream &os) const {

}*/

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    // TODO: Debug all pools
    os << "\tRegister pool:\n" << *reg_pools.back() << "\n";
    os << "\tConstant pool:\n" << *const_pool << "\n";
    os << "\tExit code: " << exit_code << "\n";
    os << "}\n";
    return os;
}

void Interpreter::run() {
    LOG1("Running interpreter");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        opc->exec(this);
        ++bci;
    }

    LOG1("Finished interpreter");
}