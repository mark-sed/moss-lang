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

BCValue *BytecodeGen::emit(ir::Expression *expr) {
    if (auto val = dyn_cast<IntLiteral>(expr)) {
        append(new StoreIntConst(next_creg(), val->get_value()));
        return last_creg();
    }
    if (auto val = dyn_cast<FloatLiteral>(expr)) {
        append(new StoreFloatConst(next_creg(), val->get_value()));
        return last_creg();
    }
    if (auto val = dyn_cast<StringLiteral>(expr)) {
        append(new StoreStringConst(next_creg(), val->get_value()));
        return last_creg();
    }
    if (auto val = dyn_cast<BoolLiteral>(expr)) {
        append(new StoreBoolConst(next_creg(), val->get_value()));
        return last_creg();
    }
    if (isa<NilLiteral>(expr)) {
        append(new StoreNilConst(next_creg()));
        return last_creg();
    }
    if (auto be = dyn_cast<BinaryExpr>(expr)) {
        return emit(be);
    }

    assert(false && "Missing Expression generation");
    return nullptr;
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