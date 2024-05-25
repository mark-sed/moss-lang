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

using Register = unsigned int;
using StringVal = ustring;

/** Base Opcode class */
class OpCode {
protected:
    Bytecode::OpCodes op_type;
    ustring mnem;

    OpCode(Bytecode::OpCodes op_type, ustring mnem) : op_type(op_type), mnem(mnem) {}
public:
    virtual ~OpCode() {}

    Bytecode::OpCodes get_type() { return this->op_type; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << mnem;
        return os;
    }
    virtual void exec() = 0;
};

inline std::ostream& operator<< (std::ostream& os, OpCode &op) {
    return op.debug(os);
}

class End : public OpCode {
public:
    static const Bytecode::OpCodes ClassType = Bytecode::OpCodes::END;

    End() : OpCode(ClassType, "END") {}
    void exec() override { /*TODO*/ }
};

class Load : public OpCode {
private:
    Register dst;
    StringVal name;
public:
    static const Bytecode::OpCodes ClassType = Bytecode::OpCodes::LOAD;

    Load(Register dst, StringVal name) : OpCode(ClassType, "LOAD"), dst(dst), name(name) {}
    void exec() override {
        // TODO
    }
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << " %" << dst << ", \"" << name << "\"";
        return os;
    }
};

/*
class Name : public OpCode {
public:
    static const Bytecode::OpCodes ClassType = Bytecode::OpCodes::NAME;

    Name() : OpCode(ClassType, "NAME") {}
    void exec() override {
        // TODO
    }
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
};
*/

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