#include "bytecodegen.hpp"
#include "logging.hpp"
#include "opcode.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;
using namespace bcgen;
using namespace opcode;

#ifndef NDEBUG
    #define comment(c) code->push_comment((c))
#else
    #define comment(c)
#endif

// TODO: Consider rewriting this without using RegValue *, but using just RegValue
RegValue *BytecodeGen::emit(ir::BinaryExpr *expr) {
    LOGMAX("Generating " << *expr);
    RegValue *left = nullptr;
    RegValue *right = nullptr;
    // Dont emit for left if set and its variable
    if (expr->get_op().get_kind() != OperatorKind::OP_SET ||
          !(isa<Variable>(expr->get_left()) || isa<OperatorLiteral>(expr->get_left()))) {
        // Dont emit also scope nor access
        BinaryExpr *be = dyn_cast<BinaryExpr>(expr->get_left());
        if (!be || be->get_op().get_kind() != OperatorKind::OP_ACCESS ||
                expr->get_op().get_kind() != OperatorKind::OP_SET) {
            left = emit(expr->get_left());
        }
    }
    if (expr->get_op().get_kind() != OperatorKind::OP_ACCESS ||
          !(isa<Variable>(expr->get_right()) || isa<OperatorLiteral>(expr->get_right()))) {
        right = emit(expr->get_right());
    }

    // TODO: Optimize 2 consts into literals
    switch (expr->get_op().get_kind()) {
        case OperatorKind::OP_CONCAT: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Concat2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Concat2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Concat3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Concat(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_EXP: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Exp2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Exp2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Exp3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Exp(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_PLUS: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Add2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Add2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Add3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Add(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_MINUS: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Sub2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Sub2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Sub3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Sub(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_DIV: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Div2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Div2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Div3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Div(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_MUL: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Mul2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Mul2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Mul3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Mul(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_MOD: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Mod2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Mod2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Mod3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Mod(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_SET: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                if (right->is_const()) {
                    append(new StoreConst(next_reg(), free_reg(right)));
                } else {
                    append(new Store(next_reg(), free_reg(right)));
                }
                if (irvar->is_non_local()) {
                    // TODO:
                    assert(false && "TODO: Storing to a non-local variable");
                }
                auto reg = last_reg();
                reg->set_silent(true);
                append(new StoreName(reg->reg(), irvar->get_name()));
                return reg;
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                auto rightE = dyn_cast<Variable>(be->get_right());
                assert(rightE && "Non assignable access");
                auto rval = last_reg();
                auto leftE = emit(be->get_left(), true);
                rval->set_silent(true);
                append(new StoreAttr(rval->reg(), free_reg(leftE), rightE->get_name()));
                return rval;
            } else {
                assert(false && "Missing assignment type");
            }
        }
        case OperatorKind::OP_SET_CONCAT: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Concat3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Concat(left->reg(), left->reg(), free_reg(right)));
            }
            // TODO: We need to store the name as the value was copied into a
            // new register and that was modified. If we don't generate load,
            // we could use just the original register and get rid of 
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_EXP: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Exp3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Exp(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_PLUS: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Add3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Add(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_MINUS: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Sub3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Sub(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_DIV: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Div3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Div(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_MUL: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Mul3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Mul(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_SET_MOD: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (irvar->is_non_local()) {
                // TODO:
                assert(false && "TODO: Storing to a non-local variable");
            }
            if (right->is_const()) {
                append(new Mod3(left->reg(), left->reg(), free_reg(right)));
            }
            else {
                append(new Mod(left->reg(), left->reg(), free_reg(right)));
            }
            append(new StoreName(left->reg(), irvar->get_name()));
            left->set_silent(true);
            return left;
        }
        case OperatorKind::OP_EQ: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Eq2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Eq2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Eq3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Eq(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_NEQ: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Neq2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Neq2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Neq3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Neq(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_BT: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Bt2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Bt2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Bt3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Bt(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_LT: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Lt2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Lt2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Lt3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Lt(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_BEQ: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Beq2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Beq2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Beq3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Beq(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_LEQ: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Leq2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Leq2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Leq3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Leq(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_SHORT_C_AND:
        case OperatorKind::OP_SHORT_C_OR:
            assert(false && "TODO: Unimplemented operator");
        case OperatorKind::OP_AND: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new And2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new And2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new And3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new And(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_OR: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Or2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Or2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Or3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Or(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_XOR: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Xor2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Xor2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Xor3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Xor(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_IN: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new In2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new In2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new In3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new In(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_ACCESS: {
            auto att = expr->get_right();
            ustring att_name = "";
            if (isa<Variable>(att)) {
                att_name = att->get_name();
            } else if (auto opl = dyn_cast<OperatorLiteral>(att)) {
                att_name = opl->get_op().as_string();
            } else {
                assert(false && "Unknown IR in access");
            }
            opcode::Register leftR = left->reg();
            assert(left);
            if (left->is_const()) {
                append(new StoreConst(next_reg(), free_reg(left)));
                leftR = val_last_reg();
                append(new LoadAttr(next_reg(), leftR, att_name));
            } else {
                append(new LoadAttr(next_reg(), free_reg(left), att_name));
            }
            return last_reg();
        }
        case OperatorKind::OP_SUBSC: {
            if (left->is_const() && right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto rightR = val_last_reg();
                append(new Subsc2(next_reg(), free_reg(left), rightR));
            }
            else if (left->is_const()) {
                append(new Subsc2(next_reg(), free_reg(left), free_reg(right)));
            }
            else if (right->is_const()) {
                append(new Subsc3(next_reg(), free_reg(left), free_reg(right)));
            }
            else {
                append(new Subsc(next_reg(), free_reg(left), free_reg(right)));
            }
            return last_reg();
        }
        case OperatorKind::OP_SCOPE: 
        case OperatorKind::OP_UNPACK:
            assert(false && "TODO: Unimplemented operator");

        default: assert(false && "Unknown binary operator");
    }

    assert(false && "Unreachable");
    return nullptr;
}

RegValue *BytecodeGen::emit(ir::UnaryExpr *expr) {
    RegValue *val = emit(expr->get_expr());

    switch (expr->get_op().get_kind()) {
        case OperatorKind::OP_NOT: {
            if (val->is_const()) {
                // TODO: This should be just output of constant
                append(new StoreConst(next_reg(), free_reg(val)));
                auto strv = val_last_reg();
                append(new Not(next_reg(), strv));
            }
            else {
                append(new Not(next_reg(), free_reg(val)));
            }
            return last_reg();
        }
        case OperatorKind::OP_MINUS: {
            if (val->is_const()) {
                // TODO: This should be just output of constant
                append(new StoreConst(next_reg(), free_reg(val)));
                auto strv = val_last_reg();
                append(new Neg(next_reg(), strv));
            }
            else {
                append(new Neg(next_reg(), free_reg(val)));
            }
            return last_reg();
        }
        case OperatorKind::OP_SILENT: {
            auto lr = last_reg();
            lr->set_silent(true);
            return lr;
        }
        default: assert(false && "Unknown unary operator");
    }

    assert(false && "Unreachable");
    return nullptr;
}

RegValue *BytecodeGen::emit(ir::Expression *expr, bool get_as_ncreg) {
    RegValue *bcv = nullptr;
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
    else if (auto val = dyn_cast<List>(expr)) {
        if (!val->is_comprehension()) {
            auto list_reg = next_reg();
            bcv = last_reg();
            append(new BuildList(list_reg));
            for (auto v: val->get_value()) {
                auto genv = emit(v);
                if (genv->is_const()) {
                    append(new ListPushConst(list_reg, free_reg(genv)));
                }
                else {
                    append(new ListPush(list_reg, free_reg(genv)));
                }
            }
        }
        else {
            assert(false && "List comprehension generation");
        } 
    }
    else if (auto be = dyn_cast<BinaryExpr>(expr)) {
        bcv = emit(be);
    }
    else if (auto ue = dyn_cast<UnaryExpr>(expr)) {
        bcv = emit(ue);
    }
    else if (auto val = dyn_cast<Variable>(expr)) {
        if (val->is_non_local()) {
            append(new LoadNonLoc(next_reg(), val->get_name()));
        }
        else {
            append(new Load(next_reg(), val->get_name()));
        }
        bcv = last_reg();
    }
    else if (isa<ThisLiteral>(expr)) {
        // This is just a variable in bytecode
        append(new Load(next_reg(), "this"));
        bcv = last_reg();
    }
    else if (auto opl = dyn_cast<OperatorLiteral>(expr)) {
        append(new Load(next_reg(), opl->get_op().as_string()));
        bcv = last_reg();
    }
    else if (auto val = dyn_cast<ir::Call>(expr)) {
        append(new PushCallFrame());
        auto fun = emit(val->get_fun());
        for (auto a: val->get_args()) {
            RegValue *a_val = nullptr;
            auto be = dyn_cast<BinaryExpr>(a);
            if (be && be->get_op().get_kind() == OperatorKind::OP_SET) {
                // Named arg
                a_val = emit(be->get_right(), true);
                ustring name = be->get_left()->get_name();
                append(new PushNamedArg(free_reg(a_val), name));
            }
            else {
                a_val = emit(a);
                if (a_val->is_const()) {
                    append(new PushConstArg(free_reg(a_val)));
                }
                else {
                    append(new PushArg(free_reg(a_val)));
                }
            }
        }
        if (auto be = dyn_cast<BinaryExpr>(val->get_fun())) {
            if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                auto ths = emit(be->get_left(), true);
                append(new PushNamedArg(free_reg(ths), "this"));
            }
        }
        append(new opcode::Call(next_reg(), free_reg(fun)));
        bcv = last_reg();
    }
    else {
        LOGMAX("MISSING EXPR ENUM: " << static_cast<int>(expr->get_type()));
        assert(false && "Missing Expression generation");
        return nullptr;
    }

    if (!get_as_ncreg || !bcv->is_const())
        return bcv;

    append(new StoreConst(next_reg(), free_reg(bcv)));
    return last_reg();
}

void BytecodeGen::emit(ir::Raise *r) {
    auto exc = emit(r->get_exception(), true);
    append(new opcode::Raise(free_reg(exc)));
}

void BytecodeGen::emit(ir::Return *r) {
    auto ex = emit(r->get_expr());
    if (ex->is_const())
        append(new opcode::ReturnConst(free_reg(ex)));
    else
        append(new opcode::Return(free_reg(ex)));
}

void BytecodeGen::emit(ir::If *ifstmt) {
    auto cond = emit(ifstmt->get_cond(), true);
    auto jmp_false = new JmpIfFalse(free_reg(cond), 0);
    append(jmp_false);
    emit(ifstmt->get_body());
    auto else_stmt = ifstmt->get_else();
    if (else_stmt) {
        auto jmp_done = new Jmp(0);
        // In case of else we have to jump after true jmp
        append(jmp_done);
        jmp_false->addr = get_curr_address() + 1;
        emit(else_stmt->get_body());
        jmp_done->addr = get_curr_address() + 1;
    }
    else {
        jmp_false->addr = get_curr_address() + 1;
    }
}

void BytecodeGen::emit(ir::While *whstmt) {
    auto pre_while_bc = get_curr_address() + 1;
    auto cond = emit(whstmt->get_cond(), true);
    auto jmp_end = new JmpIfFalse(free_reg(cond), 0);
    append(jmp_end);
    emit(whstmt->get_body());
    append(new Jmp(pre_while_bc));
    jmp_end->addr = get_curr_address() + 1;
}

void BytecodeGen::emit(ir::DoWhile *whstmt) {
    auto pre_while_bc = get_curr_address() + 1;
    emit(whstmt->get_body());
    auto cond = emit(whstmt->get_cond(), true);
    append(new JmpIfTrue(free_reg(cond), pre_while_bc));
}

void BytecodeGen::emit(ir::Module *mod) {
    emit(mod->get_body());
}

void BytecodeGen::emit(ir::Function *fun) {
    auto fun_reg = next_reg();
    auto arg_names = ir::encode_fun_args(fun->get_args());
    // Store the name without arguments
    append(new CreateFun(fun_reg, fun->get_name(), arg_names));
    comment("fun "+fun->get_name()+"(" + arg_names + ") declaration");
    // Add annotations
    for (auto annt : fun->get_annotations()) {
        auto annot_val = emit(annt->get_value(), true);
        append(new Annotate(fun_reg, annt->get_name(), free_reg(annot_val)));
    }
    // Set argument default values and types
    opcode::IntConst arg_i = 0;
    for (auto a: fun->get_args()) {
        if (a->is_vararg()) {
            append(new SetVararg(fun_reg, arg_i));
        }
        else {
            if (a->has_default_value()) {
                auto def_val = emit(a->get_default_value());
                if (def_val->is_const()) {
                    append(new SetDefaultConst(fun_reg, arg_i, free_reg(def_val)));
                }
                else {
                    append(new SetDefault(fun_reg, arg_i, free_reg(def_val)));
                }
            }
            for (auto t: a->get_types()) {
                append(new SetType(fun_reg, arg_i, t->get_name()));    
            }
        }
        ++arg_i;
    }
    append(new FunBegin(fun_reg));
    // Place jump which will be later on modified to contain the actual
    // function end - this skips beyond the function body
    auto fn_end_jmp = new Jmp(0);
    append(fn_end_jmp);
    comment("fun "+fun->get_name()+"(" + arg_names + ") body start");
    append(new PopCallFrame());
    // Registers need to be reset, store them and restore after whole
    // function is generated
    auto pre_function_reg = curr_reg;
    auto pre_function_creg = curr_creg;
    // We add one for possible "this" argument
    reset_regs(fun->get_args().size()+1);
    // Generate function body
    emit(fun->get_body());
    // TODO: Generate return in function IR body if needed, not here
    append(new StoreNilConst(next_creg()));
    append(new StoreConst(next_reg(), val_last_creg()));
    append(new opcode::Return(val_last_reg()));
    fn_end_jmp->addr = get_curr_address() + 1;

    this->curr_reg = pre_function_reg;
    this->curr_creg = pre_function_creg;
}

void BytecodeGen::emit(ir::Class *cls) {
    comment("class " + cls->get_name() + " start");
    for (auto p: cls->get_parents()) {
        auto p_reg = emit(p, true);
        append(new PushParent(free_reg(p_reg)));
    }
    append(new BuildClass(next_reg(), cls->get_name()));
    emit(cls->get_body());
    /*for (auto d : cls->get_body()) {
        if (auto be = dyn_cast<BinaryExpr>(d)) {
            if (be->get_op().get_kind() == OperatorKind::OP_SET) {
                assert(false && "TODO: Class attribute assignment");
                continue;
            }
        }
        emit(d);
    }*/
    append(new PopFrame());
    comment("class " + cls->get_name() + " end");
}

void BytecodeGen::emit(ir::Enum *enm) {
    comment("enum " + enm->get_name());
    auto list_reg = next_reg();
    append(new BuildList(list_reg));
    for (auto v: enm->get_values()) {
        auto v_reg = next_creg();
        append(new StoreStringConst(v_reg, v));
        append(new ListPushConst(list_reg, v_reg));
    }
    append(new BuildEnum(next_reg(), list_reg, enm->get_name()));
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
    else if (auto r = dyn_cast<ir::Return>(decl)) {
        emit(r);
    }
    else if (auto f = dyn_cast<ir::Function>(decl)) {
        emit(f);
    }
    else if (auto c = dyn_cast<ir::Class>(decl)) {
        emit(c);
    }
    else if (auto e = dyn_cast<ir::Enum>(decl)) {
        emit(e);
    }
    else if (auto i = dyn_cast<ir::If>(decl)) {
        emit(i);
    }
    else if (auto w = dyn_cast<ir::While>(decl)) {
        emit(w);
    }
    else if (auto w = dyn_cast<ir::DoWhile>(decl)) {
        emit(w);
    }
    else if (isa<EndOfFile>(decl)) {
        append(new End());
    }
    else {
        LOGMAX("Unimplemented IR generation for: " << *decl);
        assert(false && "Unimplemented IR generation");
    }
}

void BytecodeGen::emit(std::list<ir::IR *> block) {
    for (auto i: block) {
        emit(i);
    }
}

void BytecodeGen::output(RegValue *val) {
    // TODO: Do we need this flag? Cant we just check if the operator is not "silent"?
    // But what if it is `a || ~foo()`, then top is SHORTC_OR not SILENT
    if (!val->is_silent()) {
        val = get_ncreg(val);
        append(new Output(val->reg()));
    }
    free_reg(val);
}

void BytecodeGen::generate(ir::IR *decl) {
    LOGMAX("Generating bytecode from IR " << decl->get_name());

    emit(decl);

    LOGMAX("Finished generating bytecode");
}