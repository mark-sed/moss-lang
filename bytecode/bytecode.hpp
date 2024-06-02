/**
 * @file bytecode.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief VM bytecode
 */

#ifndef _BYTECODE_HPP_
#define _BYTECODE_HPP_

#include "opcode.hpp"
#include <cstdint>
#include <vector>

namespace moss {

namespace opcode {
    class OpCode;    
}

class Bytecode {
private:
    std::vector<opcode::OpCode *> code;
public:
    Bytecode() {}
    ~Bytecode();

    std::ostream& debug(std::ostream& os) const;

    void push_back(opcode::OpCode *op) {
        code.push_back(op);
    }

    size_t size() { return code.size(); }

    inline opcode::OpCode *operator[](uint32_t addr) {
        assert(addr < code.size() && "Out of bounds bci access");
        return code[addr];
    }

    std::vector<opcode::OpCode *> get_code() { return this->code; }

    /*std::vector<opcode::OpCode *>::iterator begin() { return code.begin(); }
    std::vector<opcode::OpCode *>::const_iterator begin() const { return code.begin(); }
    std::vector<opcode::OpCode *>::const_iterator cbegin() const { return code.cbegin(); }
    std::vector<opcode::OpCode *>::iterator end() { return code.end(); }
    std::vector<opcode::OpCode *>::const_iterator end() const { return code.end(); }
    std::vector<opcode::OpCode *>::const_iterator cend() const { return code.cend(); }*/
};

inline std::ostream& operator<< (std::ostream& os, Bytecode &bc) {
    return bc.debug(os);
}

}

#endif//_BYTECODE_HPP_