#include "bytecode.hpp"

using namespace moss;

Bytecode::~Bytecode() {
    for (auto *op: code) {
            delete op;
    }
}

std::ostream& Bytecode::debug(std::ostream& os) const {
    opcode::Address bci = 0;
    for (auto op: code) {
        os << bci++ << "\t" << *op << "\n";
    }
    return os;
}