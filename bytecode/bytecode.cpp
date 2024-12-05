#include "bytecode.hpp"
#include "clopts.hpp"

using namespace moss;

Bytecode::~Bytecode() {
    if (header)
        delete header;
    for (auto *op: code) {
        delete op;
    }
}

std::ostream& Bytecode::debug(std::ostream& os) {
    opcode::Address bci = 0;
    for (auto op: code) {
#ifndef NDEBUG
        // When --annotate-bc is set, then output comments
        if (clopts::annotate_bc && comments.find(bci) != comments.end()) {
            os << "\t; " << comments[bci] << "\n";
        }
#endif
        os << bci++ << "\t" << *op << "\n";
    }
    return os;
}