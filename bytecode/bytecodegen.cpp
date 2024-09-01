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

    switch (expr->get_op().get_kind()) {
        case OperatorKind::OP_PLUS: {
            if (left->is_const() && right->is_const()) {
                // TODO: Optimize const adds and similar into literals
                append(new StoreConst(next_reg(), left->reg()));
                auto leftR = last_reg();
                append(new Add3(next_reg(), leftR->reg(), right->reg()));
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
    else if (auto be = dyn_cast<BinaryExpr>(decl)) {
        output(emit(be));
    }
}

void BytecodeGen::output(BCValue *val) {
    if (!val->is_silent()) {
        append(new Output(free_reg(val)));
    }
}

void BytecodeGen::generate(ir::IR *decl) {
    LOGMAX("Generating bytecode from IR " << decl->get_name());

    emit(decl);

    LOGMAX("Finished generating bytecode");
}