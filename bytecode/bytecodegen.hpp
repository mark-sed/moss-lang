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

class RegValue {
private:
    opcode::Register value;
    bool constant;
    bool silent;
public:
    RegValue(opcode::Register value, bool constant=false) : value(value), constant(constant), silent(false) {}

    opcode::Register reg() { return this->value; }

    bool is_const() { return this->constant; }

    void set_reg(opcode::Register r) { this->value = r; }
    void set_const(bool c) { this->constant = c; }

    void set_silent(bool s) { this->silent = s; }
    bool is_silent() { return this->silent; }
};

}

namespace bcgen {

class BytecodeGen {
private:
    Bytecode *code;
    opcode::Register curr_creg;
    opcode::Register curr_reg;

    RegValue *emit(ir::BinaryExpr *expr);
    RegValue *emit(ir::Expression *expr, bool get_as_ncreg=false);

    void emit(ir::Raise *mod);
    void emit(ir::Module *mod);
    void emit(ir::IR *decl);

    void output(RegValue *val);

    inline void append(opcode::OpCode *opc) { code->push_back(opc); }

    inline opcode::Register next_creg() { return this->curr_creg++; }
    inline opcode::Register next_reg() { return this->curr_reg++; }

    inline RegValue *last_creg() { return new RegValue(this->curr_creg-1, true); }
    inline RegValue *last_reg() { return new RegValue(this->curr_reg-1, false); }

    inline RegValue *get_ncreg(RegValue *val) {
        assert(val && "sanity check");
        if (val->is_const()) {
            append(new opcode::StoreConst(next_reg(), val->reg()));
            val->set_reg(this->curr_reg-1);
            val->set_const(false);
        }
        return val;
    }

    inline opcode::Register free_reg(RegValue *val) {
        assert(val && "sanity check");
        auto regval = val->reg();
        delete val;
        return regval;
    }
public:
    BytecodeGen(Bytecode *code) : code(code), curr_creg(0), curr_reg(0) {}

    void generate(ir::IR *decl);
};

}

}

#endif//_BYTECODEGEN_HPP_