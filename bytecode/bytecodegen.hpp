/// 
/// \file bytecodegen.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode generator from IR
/// 

#ifndef _BYTECODEGEN_HPP_
#define _BYTECODEGEN_HPP_

#include "bytecode.hpp"
#include "ir.hpp"
#include "commons.hpp"

namespace moss {

/// Namespace for all bytecode code generation resources
namespace bcgen {

/// This class represents a register value and its properties.
/// This has to be encapsulated to store these properties and change them
/// based on other IRs. 
/// We mainly need to distinguish where this value is to be stored, if it should
/// reside in the constant pool or variable pool 
class RegValue {
private:
    opcode::Register value; ///< Actual register value for opcode
    bool constant;          ///< If the value is from constant pool
    bool silent;            ///< If the value should not be outputted
public:
    RegValue(opcode::Register value, bool constant=false) : value(value), constant(constant), silent(false) {}

    /// \return Opcode register this represents
    opcode::Register reg() { return this->value; }

    /// Constant registers reside in constant pool 
    bool is_const() { return this->constant; }

    void set_reg(opcode::Register r) { this->value = r; }
    void set_const(bool c) { this->constant = c; }

    /// Sets if the register should be outputted when left as standalone expression 
    void set_silent(bool s) { this->silent = s; }
    /// If true then register value will be outputted when it is the root register 
    bool is_silent() { return this->silent; }
};

/// \brief Bytecode generator
/// 
/// Generates bytecode from IR. This class works by modifying a Bytecode
/// class passed to it, so it keeps its state and can be called to generate
/// new code multiple times 
class BytecodeGen {
private:
    Bytecode *code;             ///< Bytecode it will be appending opcodes to
    std::list<std::pair<opcode::Register, opcode::Register>> reg_stack; ///< Stack of register pairs <reg, creg>

    /// Current free register 
    opcode::Register curr_reg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return reg_stack.back().first;
    }

    /// Current free constant register
    opcode::Register curr_creg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return reg_stack.back().second;
    }

    opcode::IntConst catch_id_counter;

    RegValue *emit(ir::BinaryExpr *expr);
    RegValue *emit(ir::UnaryExpr *expr);
    RegValue *emit(ir::Expression *expr, bool get_as_ncreg=false);

    /// \brief Emits import expression for either module or space
    /// \param e Expression to emit
    /// \param space_import This value is set within this function and used in recursive call
    void emit_import_expr(ir::Expression *e, bool space_import=false);

    /// Updates all break and continue jumps to proper addresses
    /// \param start Address where loop starts (start search)
    /// \param end Address where loop ends (end of search)
    /// \param brk Address where to jump on break
    /// \param cont Address where to jump on continue
    void update_jmps(opcode::Address start, opcode::Address end, opcode::Address brk, opcode::Address cont);

    void emit(ir::Raise *r);
    void emit(ir::Return *r);
    void emit(ir::Module *mod);
    void emit(ir::If *ifstmt);
    void emit(ir::Switch *swch);
    void emit(ir::ForLoop *forpl);
    void emit(ir::While *whstmt);
    void emit(ir::Import *im);
    void emit(ir::DoWhile *whstmt);
    void emit(ir::Try *tcf);
    void emit(ir::Function *fun);
    void emit(ir::Class *cls);
    void emit(ir::Space *spc);
    void emit(ir::Enum *enm);
    void emit(ir::Assert *asr);
    void emit(ir::Break *br);
    void emit(ir::Continue *ct);
    void emit(ir::Annotation *annt);
    void emit(ir::IR *decl);
    void emit(std::list<ir::IR *> block);

    void output(RegValue *val);

    inline void append(opcode::OpCode *opc) { code->push_back(opc); }

    inline opcode::Address get_curr_address() { return (code->empty() ? 0 : code->size()-1); }

    inline opcode::Register next_creg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return this->reg_stack.back().second++;
    }
    inline opcode::Register next_reg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return this->reg_stack.back().first++;
    }

    inline RegValue *last_creg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return new RegValue(this->reg_stack.back().second - 1, true);
    }

    inline RegValue *last_reg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return new RegValue(this->reg_stack.back().first - 1, false);
    }

    inline opcode::Register val_last_creg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return this->reg_stack.back().second - 1;
    }
    inline opcode::Register val_last_reg() {
        assert(!reg_stack.empty() && "Empty reg stack?");
        return this->reg_stack.back().first - 1;
    }

    void push_reg_scope(opcode::Register cr=0, opcode::Register ccr=0) {
        this->reg_stack.push_back({cr, ccr});
    }

    void pop_reg_scope() {
        assert(reg_stack.size() > 1 && "popping from empty or global reg scope");
        this->reg_stack.pop_back();
    }

    inline RegValue *get_ncreg(RegValue *val) {
        assert(val && "sanity check");
        if (val->is_const()) {
            append(new opcode::StoreConst(next_reg(), val->reg()));
            assert(!reg_stack.empty() && "Empty reg stack?");
            val->set_reg(this->reg_stack.back().first - 1);
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
    BytecodeGen(Bytecode *code) : code(code), catch_id_counter(0) {
        assert(code && "Generator requires a non-null Bytecode");
        reg_stack.push_back({BC_RESERVED_REGS, BC_RESERVED_CREGS});
    }
    ~BytecodeGen() {
        // Code is to be deleted by the creator of it
    }

    /// \brief Generates opcodes from IR
    /// 
    /// This modifies the Bytecode object passed to this on creation and
    /// works with this as if it was connected to the previously passed in IRs.
    /// Meaning that the registers will not start from 0 if multiple Modules
    /// are passed to this.
    /// 
    /// \note This relies on the Bytecode object (passed on creation) being
    ///       valid and not modified by anyone else but this class. 
    void generate(ir::IR *decl);
};

}

}

#endif//_BYTECODEGEN_HPP_