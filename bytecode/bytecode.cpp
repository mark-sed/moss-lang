#include "bytecode.hpp"

using namespace moss;

Bytecode::~Bytecode() {
    for (auto *op: code) {
            delete op;
    }
}

std::ostream& Bytecode::debug(std::ostream& os) const {
    for (auto op: code) {
        os << *op << "\n";
    }
    return os;
}