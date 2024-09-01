#include "bytecodegen.hpp"
#include "logging.hpp"
#include "opcode.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;
using namespace bcgen;
using namespace opcode;

BCValue *BytecodeGen::emit(ir::BinaryExpr *expr) {
    auto leftV = emit(expr->get_left());
    auto rightV = emit(expr->get_right());

    auto left = get_reg(leftV);
    auto right = get_reg(rightV);

    // TODO: Optimize 2 consts into literals
    switch (expr->get_op().get_kind()) {
        case OperatorKind::OP_CONCAT: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), right->reg()));
                auto rightR = last_reg();
                append(new Concat2(next_reg(), left->reg(), rightR->reg()));
            }
            else if (left->is_const()) {
                append(new Concat2(next_reg(), left->reg(), right->reg()));
            }
            else if (right->is_const()) {
                append(new Concat3(next_reg(), left->reg(), right->reg()));
            }
            else {
                append(new Concat(next_reg(), left->reg(), right->reg()));
            }
            return last_reg();
        }
        case OperatorKind::OP_PLUS: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), right->reg()));
                auto rightR = last_reg();
                append(new Add2(next_reg(), left->reg(), rightR->reg()));
            }
            else if (left->is_const()) {
                append(new Add2(next_reg(), left->reg(), right->reg()));
            }
            else if (right->is_const()) {
                append(new Add3(next_reg(), left->reg(), right->reg()));
            }
            else {
                append(new Add(next_reg(), left->reg(), right->reg()));
            }
            return last_reg();
        }
        default: assert(false && "Unknown operator");
    }

    assert(false && "Unreachable");
    return nullptr;
}

BCValue *BytecodeGen::emit(ir::Expression *expr, bool get_as_ncreg) {
    BCValue *bcv = nullptr;
    if (auto val = dyn_cast<IntLiteral>(expr)) {
        append(new StoreIntConst(next_creg(), val->get_value()));
        bcv = last_creg();
    }
    else if (auto val = dyn_cast<FloatLiteral>(expr)) {
        append(new StoreFloatConst(next_creg(), val->get_value()));
        bcv = last_creg();
    }
    else if (auto val = dyn_cast<StringLiteral>(expr)) {
        append(new StoreStringConst(next_creg(), val->get_value()));
        bcv = last_creg();
    }
    else if (auto val = dyn_cast<BoolLiteral>(expr)) {
        append(new StoreBoolConst(next_creg(), val->get_value()));
        bcv = last_creg();
    }
    else if (isa<NilLiteral>(expr)) {
        append(new StoreNilConst(next_creg()));
        bcv = last_creg();
    }
    else if (auto be = dyn_cast<BinaryExpr>(expr)) {
        bcv = emit(be);
    }
    else {
        assert(false && "Missing Expression generation");
        return nullptr;
    }

    if (!get_as_ncreg)
        return bcv;
    
    auto regv = dyn_cast<RegValue>(bcv);
    if (!regv->is_const())
        return bcv;
    append(new StoreConst(next_reg(), free_reg(bcv)));
    return last_reg();
}

void BytecodeGen::emit(ir::Raise *r) {
    auto exc = emit(r->get_exception(), true);
    append(new opcode::Raise(free_reg(exc)));
}

void BytecodeGen::emit(ir::Module *mod) {
    for (auto decl : mod->get_body()) {
        emit(decl);
    }
}

void BytecodeGen::emit(ir::IR *decl) {
    if (auto mod = dyn_cast<Module>(decl)) {
        emit(mod);
    }
    else if (auto e = dyn_cast<Expression>(decl)) {
        output(emit(e));
    }
    else if (auto r = dyn_cast<ir::Raise>(decl)) {
        emit(r);
    }
    else if (isa<EndOfFile>(decl)) {
        append(new End());
    }
    else {
        LOGMAX("Unimplemented IR generation for: " << *decl);
        assert(false && "Unimplemented IR generation");
    }
}

void BytecodeGen::output(BCValue *val) {
    if (!val->is_silent()) {
        auto ncreg = get_ncreg(val);
        append(new Output(ncreg->reg()));
        free_val(ncreg);
    }
    free_val(val);
}

void BytecodeGen::generate(ir::IR *decl) {
    LOGMAX("Generating bytecode from IR " << decl->get_name());

    emit(decl);

    LOGMAX("Finished generating bytecode");
}