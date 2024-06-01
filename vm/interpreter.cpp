#include "interpreter.hpp"
#include "logging.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code) : code(code) {
    this->const_pool = new MemoryPool();
    this->reg_pool = new MemoryPool();
}

Interpreter::~Interpreter() {
    delete const_pool;
    delete reg_pool;
}

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    os << "\tRegister pool:\n" << *reg_pool << "\n";
    os << "\tConstant pool:\n" << *const_pool << "\n";
    os << "}\n";
    return os;
}

void Interpreter::run() {
    LOG1("Running interpreter");

    LOG1("Finished interpreter");
}