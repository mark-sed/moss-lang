/**
 * @file opcode.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Bytecode opcodes
 */

#ifndef _OPCODE_HPP_
#define _OPCODE_HPP_

#include "os_interface.hpp"
#include "bytecode.hpp"

namespace moss {

namespace opcode {

/** Base Opcode class */
class OpCode {
protected:
    Bytecode::OpCodes op_type;
    ustring name;

    OpCode(Bytecode::OpCodes op_type, ustring name) : op_type(op_type), name(name) {}
public:
    virtual ~OpCode() {}

    Bytecode::OpCodes get_type() { return this->op_type; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<opcode>" << name;
        return os;
    }
};

inline std::ostream& operator<< (std::ostream& os, OpCode &op) {
    return op.debug(os);
}

class End : public OpCode {
public:
    static const Bytecode::OpCodes ClassType = Bytecode::OpCodes::END;
};

}

// Helper functions
template<class T>
bool isa(opcode::OpCode& o) {
    return o.get_type() == T::ClassType;
}

template<class T>
bool isa(opcode::OpCode* o) {
    return o->get_type() == T::ClassType;
}

template<class T>
T *dyn_cast(opcode::OpCode* o) {
    if (!isa<T>(o)) return nullptr;
    return dynamic_cast<T *>(o);
}

}

#endif//_OPCODE_HPP_