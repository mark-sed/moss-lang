/**
 * @file bytecodegen.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Bytecode generator from IR
 */

#ifndef _BYTECODEGEN_HPP_
#define _BYTECODEGEN_HPP_

#include "bytecode.hpp"
#include "ir.hpp"
#include "os_interface.hpp"

namespace moss {

namespace bcgen {

enum BCValueKind {
    REGISTER,
    CREGISTER,
    ADDRESS,
    STRING_CONST,
    INT_CONST,
    FLOAT_CONST,
    BOOL_CONST
};

class BCValue {
protected:
    BCValueKind kind;
    bool silent;

    BCValue(BCValueKind kind) : kind(kind), silent(false) {}
public:
    virtual ~BCValue() {}
    BCValueKind get_kind() { return this->kind; }

    void set_silent(bool s) { this->silent = s; }
    bool is_silent() { return this->silent; }
};

class RegValue : public BCValue {
private:
    opcode::Register value;
    bool constant;
public:
    static const BCValueKind ClassType = BCValueKind::REGISTER;

    RegValue(opcode::Register value, bool constant=false) : BCValue(ClassType), value(value), constant(constant) {}

    opcode::Register reg() { return this->value; }
    bool is_const() { return this->constant; }
};

}

// Helper functions for BCValues
template<class T>
bool isa(bcgen::BCValue& t) {
    return t.get_kind() == T::ClassType;
}

template<class T>
bool isa(bcgen::BCValue* t) {
    return t->get_kind() == T::ClassType;
}

template<class T>
T *dyn_cast(bcgen::BCValue* t) {
    if (!isa<T>(t)) return nullptr;
    return dynamic_cast<T *>(t);
}

namespace bcgen {

inline opcode::Register free_reg(BCValue *val) {
    auto v = dyn_cast<RegValue>(val);
    assert(v && "Register value is not a register.");
    auto regval = v->reg();
    delete v;
    return regval;
}

inline void free_val(BCValue *val) {
    assert(val);
    delete val;
}

class BytecodeGen {
private:
    Bytecode *code;
    opcode::Register curr_creg;
    opcode::Register curr_reg;

    BCValue *emit(ir::BinaryExpr *expr);
    BCValue *emit(ir::Expression *expr);

    void emit(ir::Module *mod);
    void emit(ir::IR *decl);

    void output(BCValue *val);

    inline void append(opcode::OpCode *opc) { code->push_back(opc); }

    inline opcode::Register next_creg() { return this->curr_creg++; }
    inline opcode::Register next_reg() { return this->curr_reg++; }

    inline RegValue *last_creg() { return new RegValue(this->curr_creg-1, true); }
    inline RegValue *last_reg() { return new RegValue(this->curr_reg-1, false); }

    inline RegValue *get_ncreg(BCValue *val) {
        auto v = dyn_cast<RegValue>(val);
        assert(v && "Register value is not a register.");
        if (v->is_const()) {
            append(new opcode::StoreConst(next_reg(), v->reg()));
        }
        return last_reg();
    }

    inline RegValue *get_reg(BCValue *val) {
        auto v = dyn_cast<RegValue>(val);
        assert(v && "Register value is not a register.");
        return v;
    }
public:
    BytecodeGen(Bytecode *code) : code(code), curr_creg(0), curr_reg(0) {}

    void generate(ir::IR *decl);
};

}

}

#endif//_BYTECODEGEN_HPP_