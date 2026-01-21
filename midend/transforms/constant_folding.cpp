#include "constant_folding.hpp"
#include "parser.hpp"
#include "ir.hpp"
#include "logging.hpp"
#include "opcode.hpp"
#include <cmath>

using namespace moss;
using namespace ir;

IR *ConstantFoldingPass::visit(BinaryExpr &be) {
    auto left = be.get_left();
    auto right = be.get_right();
    
    if (!left->is_constant() || !right->is_constant())
        return &be;
    
    auto opk = be.get_op().get_kind();
    // Because of `is_constant` check left and right can be only one of:
    // INT_LITERAL, FLOAT_LITERAL, BOOL_LITERAL, STRING_LITERAL, NIL_LITERAL
    auto lt = left->get_type();
    auto rt = right->get_type();

    // Nil is not folded since only == and != would be used and writing such
    // expressions is very unlikely and the time lost here checking for it is
    // not worth it.

    auto new_src_i = left->get_src_info();
    if (opk == OperatorKind::OP_CONCAT) {
        return new StringLiteral(left->as_string()+right->as_string(), new_src_i);
    } else if ((lt == IRType::INT_LITERAL || lt == IRType::FLOAT_LITERAL) &&
            (rt == IRType::FLOAT_LITERAL || rt == IRType::INT_LITERAL)) {
        // Fold Int and Float
        bool lhs_float = lt == IRType::FLOAT_LITERAL;
        bool rhs_float = rt == IRType::FLOAT_LITERAL;
        bool float_op = true;
        opcode::IntConst li, ri;
        opcode::FloatConst lf, rf;

        if (!lhs_float && !rhs_float) {
            li = dyn_cast<IntLiteral>(left)->get_value();
            ri = dyn_cast<IntLiteral>(right)->get_value();
            float_op = false;
        }
        lf = lhs_float
            ? dyn_cast<FloatLiteral>(left)->get_value()
            : dyn_cast<IntLiteral>(left)->get_value();

        rf = rhs_float
            ? dyn_cast<FloatLiteral>(right)->get_value()
            : dyn_cast<IntLiteral>(right)->get_value();

        switch(opk) {
            case OperatorKind::OP_PLUS: {
                if (float_op)
                    return new FloatLiteral(lf + rf, new_src_i);
                return new IntLiteral(li + ri, new_src_i);
            }
            case OperatorKind::OP_MINUS: {
                if (float_op)
                    return new FloatLiteral(lf - rf, new_src_i);
                return new IntLiteral(li - ri, new_src_i);
            }
            case OperatorKind::OP_EXP: {
                if (float_op)
                    return new FloatLiteral(std::pow(lf, rf), new_src_i);
                return new IntLiteral(
                    static_cast<long>(std::pow(
                        static_cast<double>(li), 
                        static_cast<double>(ri))
                    ), new_src_i);
            }
            case OperatorKind::OP_DIV: {
                if (float_op)
                    return new FloatLiteral(lf / rf, new_src_i);
                return new IntLiteral(li / ri, new_src_i);
            }
            case OperatorKind::OP_MUL: {
                if (float_op)
                    return new FloatLiteral(lf * rf, new_src_i);
                return new IntLiteral(li * ri, new_src_i);
            }
            case OperatorKind::OP_MOD: {
                if (float_op)
                    return new FloatLiteral(std::fmod(lf, rf), new_src_i);
                return new IntLiteral(li % ri, new_src_i);
            }
            case OperatorKind::OP_AND: {
                if (float_op) {
                    assert(false && "invalid literal float op not caught by parser");
                    break; // Cannot & float
                }
                return new IntLiteral(li & ri, new_src_i);
            }
            case OperatorKind::OP_OR: {
                if (float_op) {
                    assert(false && "invalid literal float op not caught by parser");
                    break;
                }
                return new IntLiteral(li | ri, new_src_i);
            }
            case OperatorKind::OP_XOR: {
                if (float_op) {
                    assert(false && "invalid literal float op not caught by parser");
                    break;
                }
                return new IntLiteral(li ^ ri, new_src_i);
            }
            case OperatorKind::OP_BT: {
                if (float_op)
                    return new BoolLiteral(lf > rf, new_src_i);
                return new BoolLiteral(li > ri, new_src_i);
            }
            case OperatorKind::OP_LT: {
                if (float_op)
                    return new BoolLiteral(lf < rf, new_src_i);
                return new BoolLiteral(li < ri, new_src_i);
            }
            case OperatorKind::OP_BEQ: {
                if (float_op)
                    return new BoolLiteral(lf >= rf, new_src_i);
                return new BoolLiteral(li >= ri, new_src_i);
            }
            case OperatorKind::OP_LEQ: {
                if (float_op)
                    return new BoolLiteral(lf <= rf, new_src_i);
                return new BoolLiteral(li <= ri, new_src_i);
            }
            case OperatorKind::OP_EQ: {
                if (float_op)
                    return new BoolLiteral(lf == rf, new_src_i);
                return new BoolLiteral(li == ri, new_src_i);
            }
            case OperatorKind::OP_NEQ: {
                if (float_op)
                    return new BoolLiteral(lf != rf, new_src_i);
                return new BoolLiteral(li != ri, new_src_i);
            }
            default:
                break;
        }
    } else if (lt == IRType::BOOL_LITERAL && rt == IRType::BOOL_LITERAL) {
        opcode::BoolConst lb = dyn_cast<BoolLiteral>(left)->get_value();
        opcode::BoolConst rb = dyn_cast<BoolLiteral>(right)->get_value();
        
        switch(opk) {
            case OperatorKind::OP_AND:
            case OperatorKind::OP_SHORT_C_AND:
                return new BoolLiteral(lb && rb, new_src_i);
            case OperatorKind::OP_OR:
            case OperatorKind::OP_SHORT_C_OR:
                return new BoolLiteral(lb || rb, new_src_i);
            case OperatorKind::OP_XOR:
                return new BoolLiteral(lb ^ rb, new_src_i);
            case OperatorKind::OP_EQ:
                return new BoolLiteral(lb == rb, new_src_i);
            case OperatorKind::OP_NEQ:
                return new BoolLiteral(lb != rb, new_src_i);
            default: break;
        }
    } else if (lt == IRType::STRING_LITERAL && rt == IRType::STRING_LITERAL) {
        opcode::StringConst ls = dyn_cast<StringLiteral>(left)->get_value();
        opcode::StringConst rs = dyn_cast<StringLiteral>(right)->get_value();

        switch(opk) {
            case OperatorKind::OP_BT:
                return new BoolLiteral(ls > rs, new_src_i);
            case OperatorKind::OP_LT:
                return new BoolLiteral(ls < rs, new_src_i);
            case OperatorKind::OP_BEQ:
                return new BoolLiteral(ls >= rs, new_src_i);
            case OperatorKind::OP_LEQ:
                return new BoolLiteral(ls <= rs, new_src_i);
            case OperatorKind::OP_EQ:
                return new BoolLiteral(ls == rs, new_src_i);
            case OperatorKind::OP_NEQ:
                return new BoolLiteral(ls != rs, new_src_i);
            case OperatorKind::OP_IN:
                return new BoolLiteral(rs.find(ls) != ustring::npos, new_src_i);
            default: break;
        }   
    } else if (lt == IRType::STRING_LITERAL && rt == IRType::INT_LITERAL && opk == OperatorKind::OP_MUL) {
        // String multiplication ("-" * 5)
        opcode::StringConst ls = dyn_cast<StringLiteral>(left)->get_value();
        opcode::IntConst ri = dyn_cast<IntLiteral>(right)->get_value();
        if (ri <= 0 || ls.empty())
            return new StringLiteral("", new_src_i);
        else if (ls.size() == 1) {
            return new StringLiteral(ustring(ri, ls[0]), new_src_i);
        } else {
            opcode::StringConst result;
            result.reserve(ls.size() * ri);
            for (opcode::IntConst i = 0; i < ri; ++i)
                result += ls;
            return new StringLiteral(result, new_src_i);
        }
    } else if (lt == IRType::STRING_LITERAL && rt == IRType::INT_LITERAL && opk == OperatorKind::OP_SUBSC) {
        // String subscript
        opcode::StringConst ls = dyn_cast<StringLiteral>(left)->get_value();
        opcode::IntConst ri = dyn_cast<IntLiteral>(right)->get_value();
        // When OOB don't fold and leave interpreter to raise exception.
        if (!opcode::is_oob(ls, ri)) {
            return new StringLiteral(opcode::str_index(ls, ri), new_src_i);
        }
        // TODO: Handle range subsc
    } else if (opk == OperatorKind::OP_EQ && lt != rt) {
        // Varying types (if not int and float which is checked above) always
        // are not equal.
        return new BoolLiteral(false, new_src_i);
    } else if (opk == OperatorKind::OP_NEQ && lt != rt) {
        return new BoolLiteral(true, new_src_i);
    }

    return &be;
}
