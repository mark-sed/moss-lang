#include "bytecode.hpp"
#include "clopts.hpp"

using namespace moss;
using namespace opcode;

Bytecode::~Bytecode() {
    if (header)
        delete header;
    for (auto *op: code) {
        delete op;
    }
}

std::ostream& Bytecode::debug(std::ostream& os, std::optional<Address> start, std::optional<Address> end) {
    Address begin = start.value_or(0);
    Address finish = end.value_or(code.size());

    // Optional: clamp to valid range
    assert(begin <= code.size() && "OOB begin");
    assert(finish <= code.size() && "OOB finish");

    for (Address bci = begin; bci < finish; ++bci) {
#ifndef NDEBUG
        // When --annotate-bc is set, then output comments
        if (clopts::annotate_bc && comments.find(bci) != comments.end()) {
            os << "\t; " << comments[bci] << "\n";
        }
#endif
        os << bci << "\t" << *code[bci] << "\n";
    }
    return os;
}