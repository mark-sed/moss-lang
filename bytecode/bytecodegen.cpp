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
          !(isa<Variable>(expr->get_left()) || isa<Multivar>(expr->get_left()) || isa<OperatorLiteral>(expr->get_left()))) {
        // Dont emit also scope nor access
        BinaryExpr *be = dyn_cast<BinaryExpr>(expr->get_left());
        if (!be || (be->get_op().get_kind() != OperatorKind::OP_ACCESS && be->get_op().get_kind() != OperatorKind::OP_SUBSC) || 
                expr->get_op().get_kind() != OperatorKind::OP_SET) {
            bool is_shc = expr->get_op().get_kind() == OperatorKind::OP_SHORT_C_AND || expr->get_op().get_kind() == OperatorKind::OP_SHORT_C_OR;
            left = emit(expr->get_left(), is_shc);
        }
    }
    if (expr->get_op().get_kind() != OperatorKind::OP_ACCESS ||
          !(isa<Variable>(expr->get_right()) || isa<OperatorLiteral>(expr->get_right()))) {
        if (expr->get_op().get_kind() != OperatorKind::OP_SHORT_C_AND && expr->get_op().get_kind() != OperatorKind::OP_SHORT_C_OR) {
            right = emit(expr->get_right());
        }
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
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new StoreConst(next_reg(), right->reg()));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new StoreNonLoc(right->reg(), irvar->get_name()));
                    }
                    right->set_silent(true);
                    return right;
                } else {
                    if (right->is_const()) {
                        append(new StoreConst(next_reg(), free_reg(right)));
                    } else {
                        append(new Store(next_reg(), free_reg(right)));
                    }
                    auto reg = last_reg();
                    reg->set_silent(true);
                    append(new StoreName(reg->reg(), irvar->get_name()));
                    return reg;
                }
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (!right->is_const() && !index->is_const()) {
                        append(new StoreSubsc(right->reg(), free_reg(leftE), free_reg(index)));
                    } else if (right->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(right->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!right->is_const() && index->is_const()){
                        append(new StoreSubscConst(right->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(right->reg(), free_reg(leftE), free_reg(index)));
                    }
                    right->set_silent(true);
                    return right;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (!right->is_const())
                        append(new StoreAttr(right->reg(), free_reg(leftE), rightE->get_name()));
                    else
                        append(new StoreConstAttr(right->reg(), free_reg(leftE), rightE->get_name()));
                    right->set_silent(true);
                    return right;
                }
                assert("Non-assignable expression");
                return nullptr;
            } else if (auto mva = dyn_cast<Multivar>(expr->get_left())) {
                if (right->is_const()) {
                    append(new opcode::StoreConst(next_reg(), free_reg(right)));
                    right = last_reg();
                }
                // Cast the value into a List to have __iter be called over objects
                //     PUSH_CALL_FRAME
                //     LOAD  %102, "List"
                //     LOAD  %103, "a"
                //     PUSH_ARG  %103
                //     CALL  %104, %102
                append(new PushCallFrame());
                auto list_reg = next_reg();
                append(new Load(list_reg, "List"));
                append(new PushArg(free_reg(right)));
                append(new opcode::Call(next_reg(), list_reg));
                right = last_reg();

                auto vars = mva->get_vars();
                if (mva->get_rest_index() > -1) {
                    // Create list of vars
                    auto vars_list = next_reg();
                    append(new BuildList(vars_list));
                    for (size_t i = 0; i < vars.size(); ++i) {
                        auto name_reg = next_reg();
                        append(new StoreName(name_reg, vars[i]->get_name()));
                        append(new opcode::StoreIntConst(next_creg(), name_reg));
                        append(new ListPushConst(vars_list, val_last_creg()));
                    }
                    append(new StoreIntConst(next_creg(), mva->get_rest_index()));
                    append(new SubscRest(vars_list, right->reg(), val_last_creg()));
                } else {
                    for (size_t i = 0; i < vars.size(); ++i) {
                        append(new opcode::StoreIntConst(next_creg(), i));
                        auto stor_reg = val_last_creg();
                        if (i == vars.size()-1)
                            append(new opcode::SubscLast(next_reg(), right->reg(), stor_reg));
                        else
                            append(new opcode::Subsc3(next_reg(), right->reg(), stor_reg));
                        append(new StoreName(val_last_reg(), vars[i]->get_name()));
                    }
                }
                right->set_silent(true);
                return right;
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new opcode::StoreConst(next_reg(), free_reg(right)));
                    right = last_reg();
                }
                append(new opcode::StoreGlobal(right->reg(), ue->get_expr()->get_name()));
                right->set_silent(true);
                return right;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_CONCAT: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Concat3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Concat(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
                    if (right->is_const()) {
                        append(new Concat3(left->reg(), left->reg(), free_reg(right)));
                    }
                    else {
                        append(new Concat(left->reg(), left->reg(), free_reg(right)));
                    }
                    append(new StoreName(left->reg(), irvar->get_name()));
                    left->set_silent(true);
                    return left;
                }
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Concat3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Concat(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Concat3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Concat(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Concat3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Concat(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_EXP: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Exp3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Exp(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Exp3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Exp(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Exp3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Exp(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Exp3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Exp(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_PLUS: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Add3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Add(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Add3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Add(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Add3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Add(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Add3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Add(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_MINUS: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Sub3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Sub(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Sub3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Sub(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Sub3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Sub(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Sub3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Sub(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_DIV: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Div3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Div(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Div3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Div(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Div3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Div(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Div3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Div(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_MUL: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Mul3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Mul(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Mul3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Mul(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Mul3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Mul(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Mul3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Mul(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
        case OperatorKind::OP_SET_MOD: {
            if (auto irvar = dyn_cast<Variable>(expr->get_left())) {
                assert(irvar && "Assigning to non-variable");
                if (irvar->is_non_local()) {
                    if (right->is_const()) {
                        append(new Mod3(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    } else {
                        append(new Mod(next_reg(), left->reg(), free_reg(right)));
                        append(new StoreNonLoc(val_last_reg(), irvar->get_name()));
                    }
                    left->set_silent(true);
                    return left;
                } else {
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
            } else if (auto be = dyn_cast<BinaryExpr>(expr->get_left())) {
                if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
                    auto index = emit(be->get_right());
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Mod3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Mod(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    if (!retv->is_const() && !index->is_const()) {
                        append(new StoreSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (retv->is_const() && !index->is_const()){
                        append(new StoreConstSubsc(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else if (!retv->is_const() && index->is_const()){
                        append(new StoreSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    } else {
                        append(new StoreConstSubscConst(retv->reg(), free_reg(leftE), free_reg(index)));
                    }
                    return retv;
                } else if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                    auto rightE = dyn_cast<Variable>(be->get_right());
                    assert(rightE && "Non assignable access");
                    auto leftE = emit(be->get_left(), true);
                    if (right->is_const())
                        append(new Mod3(next_reg(), free_reg(left), free_reg(right)));
                    else
                        append(new Mod(next_reg(), free_reg(left), free_reg(right)));
                    auto retv = last_reg();
                    retv->set_silent(true);
                    append(new StoreAttr(retv->reg(), free_reg(leftE), rightE->get_name()));
                    return retv;
                }
            } else if (auto ue = dyn_cast<UnaryExpr>(expr->get_left())) {
                assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "non-scope unary assignment");
                if (right->is_const()) {
                    append(new Mod3(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                } else {
                    append(new Mod(next_reg(), left->reg(), free_reg(right)));
                    append(new StoreGlobal(val_last_reg(), ue->get_expr()->get_name()));
                }
                left->set_silent(true);
                return left;
            } else {
                assert(false && "Missing assignment type");
            }
        } break;
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
        case OperatorKind::OP_SHORT_C_AND: {
            assert(!left->is_const() && "should have generated non-const");
            auto res_reg = next_reg();
            append(new Store(res_reg, left->reg()));
            append(new JmpIfTrue(free_reg(left), get_curr_address() + 3));
            auto jmp_end = new Jmp(0);
            append(jmp_end);
            auto right = emit(expr->get_right(), true);
            append(new Store(res_reg, free_reg(right)));
            jmp_end->addr = get_curr_address() + 1;
            return new RegValue(res_reg, false);
        }
        case OperatorKind::OP_SHORT_C_OR: {
            assert(!left->is_const() && "should have generated non-const");
            auto res_reg = next_reg();
            append(new Store(res_reg, left->reg()));
            append(new JmpIfFalse(free_reg(left), get_curr_address() + 3));
            auto jmp_end = new Jmp(0);
            append(jmp_end);
            auto right = emit(expr->get_right(), true);
            append(new Store(res_reg, free_reg(right)));
            jmp_end->addr = get_curr_address() + 1;
            return new RegValue(res_reg, false);
        }
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
            assert(false && "operator is unary");

        default: assert(false && "Unknown binary operator");
    }

    assert(false && "Unreachable");
    return nullptr;
}

RegValue *BytecodeGen::emit(ir::UnaryExpr *expr) {
    RegValue *val = nullptr;
    // For global value dont output the value access
    if (expr->get_op().get_kind() != OperatorKind::OP_SCOPE)
        val = emit(expr->get_expr());

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
        case OperatorKind::OP_SCOPE: {
            append(new LoadGlobal(next_reg(), expr->get_expr()->get_name()));
            return last_reg();
        }
        case OperatorKind::OP_UNPACK: {
            if (val->is_const()) {
                append(new StoreConst(next_reg(), val->reg()));
                append(new PushUnpacked(val_last_reg()));
            } else {
                append(new PushUnpacked(val->reg()));
            }
            val->set_silent(true);
            return val;
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
            auto generator = val->as_for();
            emit(generator);
            append(new opcode::Load(next_reg(), val->get_compr_result_name()));
            return last_reg();
        }
    }
    else if (auto val = dyn_cast<Dict>(expr)) {
        auto key_list_reg = next_reg();
        append(new BuildList(key_list_reg));
        for (auto v: val->get_keys()) {
            auto genv = emit(v);
            if (genv->is_const()) {
                append(new ListPushConst(key_list_reg, free_reg(genv)));
            }
            else {
                append(new ListPush(key_list_reg, free_reg(genv)));
            }
        }
        auto vals_list_reg = next_reg();
        append(new BuildList(vals_list_reg));
        for (auto v: val->get_values()) {
            auto genv = emit(v);
            if (genv->is_const()) {
                append(new ListPushConst(vals_list_reg, free_reg(genv)));
            }
            else {
                append(new ListPush(vals_list_reg, free_reg(genv)));
            }
        }
        append(new BuildDict(next_reg(), key_list_reg, vals_list_reg));
        bcv = last_reg();
    }
    else if (auto nt = dyn_cast<Note>(expr)) {
        auto note = emit(nt->get_note());
        assert(note->is_const() && "String literal is not const");
        append(new PushCallFrame());
        append(new PushConstArg(free_reg(note)));
        append(new opcode::CallFormatter(next_reg(), nt->get_prefix()));
        bcv = last_reg();
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
    else if (isa<SuperLiteral>(expr)) {
        // Super is just a variable in bytecode
        append(new Load(next_reg(), "super"));
        bcv = last_reg();
    }
    else if (auto opl = dyn_cast<OperatorLiteral>(expr)) {
        append(new Load(next_reg(), opl->get_op().as_string()));
        bcv = last_reg();
    }
    else if (auto val = dyn_cast<ir::Call>(expr)) {
        append(new PushCallFrame());
        auto fun = emit(val->get_fun(), true);
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
                auto a_unary = dyn_cast<UnaryExpr>(a); 
                if (!a_unary || a_unary->get_op().get_kind() != OperatorKind::OP_UNPACK) {
                    if (a_val->is_const()) {
                        append(new PushConstArg(free_reg(a_val)));
                    }
                    else {
                        append(new PushArg(free_reg(a_val)));
                    }
                }
            }
        }
        if (auto be = dyn_cast<BinaryExpr>(val->get_fun())) {
            // We push "this" always, but remove it if it should not be passed in
            if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
                auto ths = emit(be->get_left(), true);
                append(new PushNamedArg(free_reg(ths), "this"));
            }
        }
        append(new opcode::Call(next_reg(), free_reg(fun)));
        bcv = last_reg();
    }
    else if (auto rng = dyn_cast<ir::Range>(expr)) {
        auto start = rng->get_start();
        auto second = rng->get_second();
        auto end = rng->get_end();
        assert(start && "sanity check");
        assert(end && "sanity check");

        auto start_reg = emit(start);
        auto end_reg = emit(end);
        RegValue *step_reg = nullptr;
        // Step has to be calculated from start and second
        if (second) {
            auto sec_reg = emit(second);
            if (start_reg->is_const()) {
                if (sec_reg->is_const()) {
                    append(new StoreConst(next_reg(), free_reg(sec_reg)));
                    sec_reg = last_reg();
                }
                append(new Sub3(next_reg(), free_reg(sec_reg), start_reg->reg()));
            } else {
                if (sec_reg->is_const()) {
                    append(new Sub2(next_reg(), free_reg(sec_reg), start_reg->reg()));
                } else {
                    append(new Sub(next_reg(), free_reg(sec_reg), start_reg->reg()));
                }
            }
            step_reg = last_reg();
        } else {
            // If step is not set, then set it to nil
            append(new StoreNilConst(next_creg()));
            step_reg = last_creg();
        }

        if (!start_reg->is_const() && !step_reg->is_const() && !end_reg->is_const())
            append(new CreateRange(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (start_reg->is_const() && !step_reg->is_const() && !end_reg->is_const())
            append(new CreateRange2(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (!start_reg->is_const() && step_reg->is_const() && !end_reg->is_const())
            append(new CreateRange3(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (!start_reg->is_const() && !step_reg->is_const() && end_reg->is_const())
            append(new CreateRange4(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (start_reg->is_const() && step_reg->is_const() && !end_reg->is_const())
            append(new CreateRange5(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (start_reg->is_const() && !step_reg->is_const() && end_reg->is_const())
            append(new CreateRange6(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (!start_reg->is_const() && step_reg->is_const() && end_reg->is_const())
            append(new CreateRange7(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else if (start_reg->is_const() && step_reg->is_const() && end_reg->is_const())
            append(new CreateRange8(next_reg(), free_reg(start_reg), free_reg(step_reg), free_reg(end_reg)));
        else {
            assert(false && "unreachable");
        }
        bcv = last_reg();
    }
    else if (auto tif = dyn_cast<ir::TernaryIf>(expr)) {
        auto cond = emit(tif->get_condition(), true);
        auto jmp_false = new JmpIfFalse(free_reg(cond), 0);
        auto res_reg = next_reg();
        append(jmp_false);
        auto tr_val = emit(tif->get_value_true());
        if (tr_val->is_const()) {
            append(new StoreConst(res_reg, free_reg(tr_val)));
        } else {
            append(new Store(res_reg, free_reg(tr_val)));
        }
        auto jmp_end = new Jmp(0);
        append(jmp_end);
        jmp_false->addr = get_curr_address() + 1;
        auto fl_val = emit(tif->get_value_false());
        if (fl_val->is_const()) {
            append(new StoreConst(res_reg, free_reg(fl_val)));
        } else {
            append(new Store(res_reg, free_reg(fl_val)));
        }
        jmp_end->addr = get_curr_address() + 1;
        bcv = new RegValue(res_reg, false);
    }
    else if (auto lmb = dyn_cast<ir::Lambda>(expr)) {
        auto fun_reg = next_reg();
        auto arg_names = ir::encode_fun_args(lmb->get_args());
        // Store the name without arguments
        append(new CreateFun(fun_reg, lmb->get_name(), arg_names));
        comment("lambda fun "+lmb->get_name()+"(" + arg_names + ") declaration");
        // Add annotations
        for (auto annt : lmb->get_annotations()) {
            auto annot_val = emit(annt->get_value(), true);
            append(new Annotate(fun_reg, annt->get_name(), free_reg(annot_val)));
        }
        // Set argument default values and types
        opcode::IntConst arg_i = 0;
        for (auto a: lmb->get_args()) {
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
                    auto t_reg = emit(t, true);
                    append(new SetType(fun_reg, arg_i, free_reg(t_reg)));    
                }
            }
            ++arg_i;
        }
        append(new FunBegin(fun_reg));
        // Place jump which will be later on modified to contain the actual
        // function end - this skips beyond the function body
        auto fn_end_jmp = new Jmp(0);
        append(fn_end_jmp);
        comment("lambda fun "+lmb->get_name()+"(" + arg_names + ") body start");
        append(new PopCallFrame());
        // Registers need to be reset, store them and restore after whole
        // function is generated
        auto pre_function_reg = curr_reg;
        auto pre_function_creg = curr_creg;
        // We add one for possible "this" argument
        reset_regs(lmb->get_args().size()+1);
        // Generate function body
        auto rval = emit(lmb->get_body());
        if (rval->is_const())
            append(new opcode::ReturnConst(free_reg(rval)));
        else
            append(new opcode::Return(free_reg(rval)));
        fn_end_jmp->addr = get_curr_address() + 1;

        this->curr_reg = pre_function_reg;
        this->curr_creg = pre_function_creg;
        bcv = new RegValue(fun_reg, false);
        bcv->set_silent(true);
    }
    else if (isa<Multivar>(expr)) {
        assert(false && "Standalone multivar?");
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

void BytecodeGen::emit(ir::Switch *swch) {
    auto cond = emit(swch->get_cond(), true);
    auto val_list = next_reg();
    auto addr_list = next_reg();
    auto body = swch->get_body();
    append(new opcode::BuildList(val_list));

    std::vector<ir::Case *> cases;
    for (auto i: body) {
        auto c = dyn_cast<ir::Case>(i);
        assert(c && "Non-case value in switch");
        cases.push_back(c);
        for (auto cv: c->get_values()) {
            auto v = emit(cv);
            if (v->is_const())
                append(new opcode::ListPushConst(val_list, free_reg(v)));
            else
                append(new opcode::ListPush(val_list, free_reg(v)));
        }
    }
    append(new opcode::BuildList(addr_list));

    std::vector<opcode::StoreIntConst *> addr_stores;
    for (auto c: cases) {
        for (size_t i = 0; i < c->get_values().size(); ++i) {
            auto a = new opcode::StoreIntConst(next_creg(), 0); 
            addr_stores.push_back(a);
            append(a);
            append(new opcode::ListPushConst(addr_list, val_last_creg()));
        }
    }

    auto swch_op = new opcode::Switch(free_reg(cond), val_list, addr_list, 0);
    append(swch_op);

    std::vector<opcode::Jmp *> end_jmps;
    // Body generation
    size_t addr_index = 0;
    for (auto c: cases) {
        if (c->is_default_case()) {
            swch_op->default_addr = get_curr_address() + 1;
        } else {
            for (size_t i = 0; i < c->get_values().size(); ++i) {
                // Set addresses as there are duplicate values, but only 1 body
                // We store the address as if it was a register
                addr_stores[addr_index++]->val = get_curr_address() + 1;
            }
        }
        emit(c->get_body());
        auto jmp = new opcode::Jmp(0);
        end_jmps.push_back(jmp);
        append(jmp);
    }

    auto end_addr = get_curr_address() + 1;
    for (auto ej : end_jmps) {
        ej->addr = end_addr;
    }
    // default addr cannot be 0 as there are list generations and such in the
    // very least
    if (swch_op->default_addr == 0)
        swch_op->default_addr = end_addr;
}

void BytecodeGen::update_jmps(Address start, Address end, Address brk, Address cont) {
    // Go through and update all break and continue jumps
    for (auto bci = start; bci < end; bci++) {
        if (auto j = dyn_cast<Jmp>(code->get_code()[bci])) {
            if (j->state == Jmp::JMPState::NOT_SET_BREAK) {
                j->addr = brk;
                j->state = Jmp::JMPState::SET;
            } else if (j->state == Jmp::JMPState::NOT_SET_CONTINUE) {
                j->addr = cont;
                j->state = Jmp::JMPState::SET;
            }
        }
    }
}

void BytecodeGen::emit(ir::While *whstmt) {
    auto pre_while_bc = get_curr_address() + 1;
    auto cond = emit(whstmt->get_cond(), true);
    auto jmp_end = new JmpIfFalse(free_reg(cond), 0);
    append(jmp_end);
    emit(whstmt->get_body());
    update_jmps(pre_while_bc, get_curr_address()+1, get_curr_address()+2, pre_while_bc);
    append(new Jmp(pre_while_bc));
    jmp_end->addr = get_curr_address() + 1;
}

void BytecodeGen::emit(ir::DoWhile *whstmt) {
    auto pre_while_bc = get_curr_address() + 1;
    emit(whstmt->get_body());
    auto cond_bc = get_curr_address() + 1;
    auto cond = emit(whstmt->get_cond(), true);
    append(new JmpIfTrue(free_reg(cond), pre_while_bc));
    update_jmps(pre_while_bc, get_curr_address(), get_curr_address() + 1, cond_bc);
}

void BytecodeGen::emit(ir::ForLoop *forlp) {
    Register iter = 0;
    auto i_expr = forlp->get_iterator();
    auto mva = dyn_cast<Multivar>(i_expr);
    if (isa<Variable>(i_expr)) {
        iter = next_reg();
    } else if (mva) {
        iter = next_reg();
        append(new BuildList(iter));
        for (size_t i = 0; i < mva->get_vars().size(); ++i) {
            auto name_reg = next_reg();
            append(new StoreNilConst(next_creg()));
            append(new StoreConst(name_reg, val_last_creg()));
            append(new StoreName(name_reg, mva->get_vars()[i]->get_name()));
            append(new opcode::StoreIntConst(next_creg(), name_reg));
            append(new ListPushConst(iter, val_last_creg()));
        }
    } else {
        auto iter_reg = emit(i_expr, true);
        iter = free_reg(iter_reg);
    }
    if (!mva) {
        append(new StoreNilConst(next_creg()));
        append(new StoreConst(iter, val_last_creg()));
        append(new StoreName(iter, forlp->get_iterator()->get_name()));
    }
    auto collection = emit(forlp->get_collection(), true);
    auto new_iter = next_reg();
    append(new opcode::Iter(new_iter, free_reg(collection)));
    auto pre_for_bc = get_curr_address() + 1;
    if (!mva) {
        auto for_op = new opcode::For(iter, new_iter, 0);
        append(for_op);
        emit(forlp->get_body());
        update_jmps(pre_for_bc, get_curr_address()+1, get_curr_address()+2, pre_for_bc);
        append(new Jmp(pre_for_bc));
        for_op->addr = get_curr_address() + 1;
    } else {
        auto unpack_reg = next_creg();
        append(new StoreIntConst(unpack_reg, mva->get_rest_index()));
        auto for_op = new opcode::ForMulti(iter, new_iter, 0, unpack_reg);
        append(for_op);
        emit(forlp->get_body());
        update_jmps(pre_for_bc, get_curr_address()+1, get_curr_address()+2, pre_for_bc);
        append(new Jmp(pre_for_bc));
        for_op->addr = get_curr_address() + 1;
    }
}

void BytecodeGen::emit_import_expr(ir::Expression *e, bool space_import) {
    if (auto vr = dyn_cast<ir::Variable>(e)) {
        if (!space_import)
            append(new opcode::Import(next_reg(), e->get_name()));
        else {
            if (vr->is_non_local())
                append(new opcode::LoadNonLoc(next_reg(), e->get_name()));
            else
                append(new opcode::Load(next_reg(), e->get_name()));
        }
    } else if (auto be = dyn_cast<BinaryExpr>(e)) {
        assert(be->get_op().get_kind() == OperatorKind::OP_ACCESS && "Incorrect expression in import");
        emit_import_expr(be->get_left(), space_import);
        auto res_reg = val_last_reg();
        if (isa<AllSymbols>(be->get_right())) {
            append(new opcode::ImportAll(res_reg));
        } else {
            append(new opcode::LoadAttr(next_reg(), res_reg, be->get_right()->get_name()));
        }
    } else if (auto ue = dyn_cast<UnaryExpr>(e)) {
        // Possible space import (import ::S.*)
        assert(ue->get_op().get_kind() == OperatorKind::OP_SCOPE && "Incorrect expression in import");
        assert(!isa<AllSymbols>(ue->get_expr()) && "Cannot import from this scope into this scope");
        if (space_import) {
            // import of global space `::(::S.*)`
            auto vr = dyn_cast<Variable>(ue->get_expr());
            assert(vr && "incorrect space import");
            assert(!vr->is_non_local() && "Cannot have non-local global value");
            append(new opcode::LoadGlobal(next_reg(), vr->get_name()));
            return;
        } else {
            emit_import_expr(ue->get_expr(), true);
        }
    } else {
        assert(false && "Incorrect import expression value");
    }
}

void BytecodeGen::emit(ir::Import *im) {
    assert(im->get_names().size() == im->get_aliases().size() && "we should always set alias");
    for (size_t i = 0; i < im->get_names().size(); ++i) {
        emit_import_expr(im->get_names()[i]);
        if (!isa<ImportAll>(code->get_code().back()))
            append(new StoreName(val_last_reg(), im->get_aliases()[i]));
    }
}

void BytecodeGen::emit(ir::Module *mod) {
    if (!mod->get_documentation().empty()) {
        auto doc_var = next_reg();
        auto str_var = next_creg();
        append(new StoreStringConst(str_var, mod->get_documentation()));
        append(new StoreConst(doc_var, str_var));
        append(new StoreName(doc_var, "__doc"));
    }
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
    if (!fun->get_documentation().empty()) {
        append(new Document(fun_reg, fun->get_documentation()));
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
                auto t_reg = emit(t, true);
                append(new SetType(fun_reg, arg_i, free_reg(t_reg)));    
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
    // Add annotations
    // Since BuildClass will push a new frame we need to load the class by name
    auto cls_reg = next_reg();
    append(new Load(cls_reg, cls->get_name()));
    for (auto annt : cls->get_annotations()) {
        auto annot_val = emit(annt->get_value(), true);
        append(new Annotate(cls_reg, annt->get_name(), free_reg(annot_val)));
    }
    if (!cls->get_documentation().empty()) {
        append(new Document(cls_reg, cls->get_documentation()));
    }
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

void BytecodeGen::emit(ir::Space *spc) {
    comment("space " + spc->get_name() + " start");
    append(new BuildSpace(next_reg(), spc->get_name(), spc->is_anonymous()));
    // Add annotations
    // Since BuildSpace will push a new frame we need to load the space by name
    auto spc_reg = next_reg();
    append(new Load(spc_reg, spc->get_name()));
    for (auto annt : spc->get_annotations()) {
        auto annot_val = emit(annt->get_value(), true);
        append(new Annotate(spc_reg, annt->get_name(), free_reg(annot_val)));
    }
    if (!spc->get_documentation().empty()) {
        append(new Document(spc_reg, spc->get_documentation()));
    }
    emit(spc->get_body());

    append(new PopFrame());
    comment("space " + spc->get_name() + " end");
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

void BytecodeGen::emit(ir::Try *tcf) {
    // Vector of catches that were generated to set jumps to their blocks
    // It is 2D to handle typed arguments
    std::vector<std::vector<opcode::OpCode *>> gen_catches{};
    unsigned catch_am = 0;
    for (auto riter = tcf->get_catches().rbegin(); riter != tcf->get_catches().rend(); ++riter) {
        auto ctch = *riter; 
        Argument *a = ctch->get_arg();
        assert(!a->has_default_value() && "Somehow catch argument has default value");
        assert(!a->is_vararg() && "Somehow catch argument is vararg");
        if (a->is_typed()) {
            std::vector<opcode::OpCode *> typed_catches{};
            for (auto t: a->get_types()) {
                auto type_reg = emit(t, true);
                auto c = new opcode::CatchTyped(a->get_name(), free_reg(type_reg), 0);
                typed_catches.push_back(c);
                append(c);
                ++catch_am;
            }
            gen_catches.push_back(typed_catches);
        } else {
            auto c = new opcode::Catch(a->get_name(), 0);
            gen_catches.push_back(std::vector<opcode::OpCode *>{c});
            append(c);
            ++catch_am;
        }
    }

    // Try body
    emit(tcf->get_body());
    auto try_jmp = new opcode::Jmp(0);
    append(try_jmp);

    // List of jumps in catch blocks to set their addresses to finally block
    std::vector<opcode::Jmp *> jmps{};
    unsigned i = 0;
    for (auto riter = tcf->get_catches().rbegin(); riter != tcf->get_catches().rend(); ++riter) {
        auto ctch = *riter; 
        if (auto c = dyn_cast<opcode::Catch>(gen_catches[i][0])) {
            c->addr = get_curr_address() + 1;
        } else {
            for (auto cv: gen_catches[i]) {
                auto ct = dyn_cast<opcode::CatchTyped>(cv);
                assert(ct && "Non catch was pushed into catch list");
                ct->addr = get_curr_address() + 1;
            }
        }
        
        emit(ctch->get_body());

        auto j = new opcode::Jmp(0);
        jmps.push_back(j);
        append(j);
        ++i;
    }

    for (auto j: jmps) {
        j->addr = get_curr_address() + 1;
    }
    // If try succeeds it needs to jump after catches (which might be finally)
    try_jmp->addr = get_curr_address() + 1;

    // Finally generation
    if (auto fnl = tcf->get_finally()) {
        emit(fnl->get_body());
    }

    // Generating pop_catch
    for (unsigned i = 0; i < catch_am; ++i) {
        append(new opcode::PopCatch());
    }
}

void BytecodeGen::emit(ir::Assert *asr) {
    auto cnd = emit(asr->get_cond(), true);
    RegValue *msg = nullptr;
    if (asr->get_msg())
        msg = emit(asr->get_msg(), true);
    else {
        // TODO: Load prestored empty string
        append(new StoreStringConst(next_creg(), ""));
        append(new StoreConst(next_reg(), val_last_creg()));
        msg = last_reg();
    }
    append(new opcode::Assert(free_reg(cnd), free_reg(msg)));
}

void BytecodeGen::emit(ir::Break *br) {
    (void)br;
    append(new opcode::Jmp(0, opcode::Jmp::JMPState::NOT_SET_BREAK));
}

void BytecodeGen::emit(ir::Continue *ct) {
    (void)ct;
    append(new opcode::Jmp(0, opcode::Jmp::JMPState::NOT_SET_CONTINUE));
}

void BytecodeGen::emit(ir::Annotation *annt) {
    auto annot_val = emit(annt->get_value(), true);
    append(new AnnotateMod(annt->get_name(), free_reg(annot_val)));
}

void BytecodeGen::emit(ir::IR *decl) {
    assert(decl && "emitting nullptr");
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
    else if (auto s = dyn_cast<ir::Space>(decl)) {
        emit(s);
    }
    else if (auto e = dyn_cast<ir::Enum>(decl)) {
        emit(e);
    }
    else if (auto i = dyn_cast<ir::If>(decl)) {
        emit(i);
    }
    else if (auto s = dyn_cast<ir::Switch>(decl)) {
        emit(s);
    }
    else if (auto f = dyn_cast<ir::ForLoop>(decl)) {
        emit(f);
    }
    else if (auto w = dyn_cast<ir::While>(decl)) {
        emit(w);
    }
    else if (auto w = dyn_cast<ir::DoWhile>(decl)) {
        emit(w);
    }
    else if (auto i = dyn_cast<ir::Import>(decl)) {
        emit(i);
    }
    else if (auto t = dyn_cast<ir::Try>(decl)) {
        emit(t);
    }
    else if (auto a = dyn_cast<ir::Assert>(decl)) {
        emit(a);
    }
    else if (auto b = dyn_cast<ir::Break>(decl)) {
        emit(b);
    }
    else if (auto c = dyn_cast<ir::Continue>(decl)) {
        emit(c);
    }
    else if (auto a = dyn_cast<ir::Annotation>(decl)) {
        emit(a);
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