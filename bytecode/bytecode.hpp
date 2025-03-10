///
/// \file bytecode.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief VM bytecode
///

#ifndef _BYTECODE_HPP_
#define _BYTECODE_HPP_

#include "opcode.hpp"
#include "bytecode_header.hpp"
#include <cstdint>
#include <vector>
#include <map>

namespace moss {

namespace opcode {
    class OpCode;    
}

/// \brief Class holding bytecode program
/// It consists of a vector of opcodes and API to work with it. 
class Bytecode {
private:
    std::vector<opcode::OpCode *> code;
    bc_header::BytecodeHeader *header;
#ifndef NDEBUG
    std::map<unsigned, ustring> comments;
#endif
public:
    Bytecode() : header(nullptr) {}
    ~Bytecode();

    std::ostream& debug(std::ostream& os);

    void push_back(opcode::OpCode *op) {
        code.push_back(op);
    }

    bool empty() { return code.empty(); }

#ifndef NDEBUG
    void push_comment(ustring comm) { comments[code.size()-1] = comm; }
#endif

    /// How many opcodes are in this bytecode program
    size_t size() { return code.size(); }

    inline opcode::OpCode *operator[](opcode::Address addr) {
        assert(addr < code.size() && "Out of bounds bci access");
        return code[addr];
    }

    std::vector<opcode::OpCode *> get_code() { return this->code; }

    void set_header(bc_header::BytecodeHeader *header) {
        this->header = header;
    }

    bc_header::BytecodeHeader *get_header() {
        return this->header;
    }
};

inline std::ostream& operator<< (std::ostream& os, Bytecode &bc) {
    return bc.debug(os);
}

}

#endif//_BYTECODE_HPP_