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
#include "commons.hpp"

namespace moss {

/** Namespace for all bytecode code generation resources */
namespace bcgen {

/**
 * This class represents a register value and its properties.
 * This has to be encapsulated to store these properties and change them
 * based on other IRs. 
 * We mainly need to distinguish where this value is to be stored, if it should
 * reside in the constant pool or variable pool
 */
class RegValue {
private:
    opcode::Register value; ///< Actual register value for opcode
    bool constant;          ///< If the value is from constant pool
    bool silent;            ///< If the value should not be outputted
public:
    RegValue(opcode::Register value, bool constant=false) : value(value), constant(constant), silent(false) {}

    /** @return Opcode register this represents */
    opcode::Register reg() { return this->value; }

    bool is_const() { return this->constant; }

    void set_reg(opcode::Register r) { this->value = r; }
    void set_const(bool c) { this->constant = c; }

    void set_silent(bool s) { this->silent = s; }
    bool is_silent() { return this->silent; }
};

/**
 * @brief Bytecode generator
 * 
 * Generates bytecode from IR. This class works by modifying a Bytecode
 * class passed to it, so it keeps its state and can be called to generate
 * new code multiple times
 */
class BytecodeGen {
private:
    Bytecode *code; ///< Bytecode it will be appending opcodes to
    opcode::Register curr_creg; ///< Current free constant register
    opcode::Register curr_reg;  ///< Current free register

    RegValue *emit(ir::BinaryExpr *expr);
    RegValue *emit(ir::UnaryExpr *expr);
    RegValue *emit(ir::Expression *expr, bool get_as_ncreg=false);

    void emit(ir::Raise *r);
    void emit(ir::Return *r);
    void emit(ir::Module *mod);
    void emit(ir::If *ifstmt);
    void emit(ir::While *whstmt);
    void emit(ir::DoWhile *whstmt);
    void emit(ir::Function *fun);
    void emit(ir::Class *cls);
    void emit(ir::Enum *enm);
    void emit(ir::IR *decl);
    void emit(std::list<ir::IR *> block);

    void output(RegValue *val);

    inline void append(opcode::OpCode *opc) { code->push_back(opc); }

    inline opcode::Address get_curr_address() { return (code->empty() ? 0 : code->size()-1); }

    inline opcode::Register next_creg() { return this->curr_creg++; }
    inline opcode::Register next_reg() { return this->curr_reg++; }

    inline RegValue *last_creg() { return new RegValue(this->curr_creg-1, true); }
    inline RegValue *last_reg() { return new RegValue(this->curr_reg-1, false); }

    inline opcode::Register val_last_creg() { return this->curr_creg-1; }
    inline opcode::Register val_last_reg() { return this->curr_reg-1; }

    void reset_regs(opcode::Register cr=0) {
        curr_reg = cr;
    }

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
    BytecodeGen(Bytecode *code) : code(code), curr_creg(BC_RESERVED_CREGS), curr_reg(BC_RESERVED_REGS) {
        assert(code && "Generator requires a non-null Bytecode");
    }
    ~BytecodeGen() {
        // Code is to be deleted by the creator of it
    }

    /**
     * @brief Generates opcodes from IR
     * 
     * This modifies the Bytecode object passed to this on creation and
     * works with this as if it was connected to the previously passed in IRs.
     * Meaning that the registers will not start from 0 if multiple Modules
     * are passed to this.
     * 
     * @note This relies on the Bytecode object (passed on creation) being
     *       valid and not modified by anyone else but this class.
     */
    void generate(ir::IR *decl);
};

}

}

#endif//_BYTECODEGEN_HPP_