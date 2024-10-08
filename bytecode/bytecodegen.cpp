#include "bytecodegen.hpp"
#include "logging.hpp"
#include "opcode.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;
using namespace bcgen;
using namespace opcode;

// TODO: Consider rewriting this without using RegValue *, but using just RegValue
RegValue *BytecodeGen::emit(ir::BinaryExpr *expr) {
    // Dont emit for left if set and its variable
    RegValue *left = nullptr;
    if (expr->get_op().get_kind() != OperatorKind::OP_SET || !isa<Variable>(expr->get_left()))
        left = emit(expr->get_left());
    auto right = emit(expr->get_right());

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
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
            if (right->is_const()) {
                append(new StoreConst(next_reg(), free_reg(right)));
                auto reg = last_reg();
                reg->set_silent(true);
                append(new StoreName(reg->reg(), irvar->get_name()));
                return reg;
            }
            else {
                append(new Store(next_reg(), free_reg(right)));
                auto reg = last_reg();
                reg->set_silent(true);
                append(new StoreName(reg->reg(), irvar->get_name()));
                return reg;
            }
        }
        case OperatorKind::OP_SET_CONCAT: {
            auto irvar = dyn_cast<Variable>(expr->get_left());
            assert(irvar && "Assigning to non-variable");
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
        case OperatorKind::OP_ACCESS:
            assert(false && "TODO: Unimplemented operator");
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

        default: assert(false && "Unknown operator");
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
    else if (auto be = dyn_cast<BinaryExpr>(expr)) {
        bcv = emit(be);
    }
    else if (auto val = dyn_cast<Variable>(expr)) {
        append(new Load(next_reg(), val->get_name()));
        bcv = last_reg();
    }
    else {
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

void BytecodeGen::emit(ir::Module *mod) {
    for (auto decl : mod->get_body()) {
        emit(decl);
    }
}

void BytecodeGen::emit(ir::Function *fun) {
    auto fun_reg = next_reg();
    auto names = ir::encode_fun(fun);
    assert(!names.empty() && "No names generated for a function");
    // Assert names to the function
    for (auto n: names) {
        append(new StoreName(fun_reg, n));
    }
    // Store into the function register the address of push_frame opcode
    // The curr_address is store name, next is store addr (+1), then
    // jmp (+2) and after that push frame (+3)
    append(new StoreAddr(fun_reg, get_curr_address()+3));
    // Place jump which will be later on modified to contain the actual
    // function end - this skips beyond the function body
    auto fn_end_jmp = new Jmp(0);
    append(fn_end_jmp);
    append(new PushFrame());
    // Generate function body
    for (auto decl : fun->get_body()) {
        emit(decl);
    }
    // TODO: Generate return in function IR body if needed, not here
    append(new StoreNilConst(next_creg()));
    append(new StoreConst(next_reg(), val_last_creg()));
    append(new opcode::Return(val_last_reg()));
    fn_end_jmp->addr = get_curr_address() + 1;
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
    else if (auto f = dyn_cast<ir::Function>(decl)) {
        emit(f);
    }
    else if (isa<EndOfFile>(decl)) {
        append(new End());
    }
    else {
        LOGMAX("Unimplemented IR generation for: " << *decl);
        assert(false && "Unimplemented IR generation");
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