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
};

inline std::ostream& operator<< (std::ostream& os, Bytecode &bc) {
    return bc.debug(os);
}

}

#endif//_BYTECODE_HPP_