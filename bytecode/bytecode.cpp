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

void Bytecode::insert(opcode::OpCode *op, opcode::Address bci) {
    assert(bci <= code.size() && "Inserting way after last BC");
    if (bci == code.size()) {
        push_back(op);
        return;
    }
    for (auto i: code) {
        i->update_addrs(bci, 1);
    }
    code.insert(code.begin() + bci, op);
}

void Bytecode::erase(Address bci) {
    assert(bci < code.size() && "Deleting bci out of bounds");
    OpCode *op = code[bci];
    // No need to optimize for popping last, since it is END and there is no
    // reason to pop it.
    code.erase(code.begin() + bci);
    for (auto i: code) {
        i->update_addrs(bci, -1);
    }
    assert(op && "Sanity check");
    delete op;
}