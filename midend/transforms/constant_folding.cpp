#include "constant_folding.hpp"
#include "parser.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

IR *ConstantFoldingPass::visit(BinaryExpr &be) {
    auto left = be.get_left();
    auto right = be.get_right();
    
    // TODO: List maybe could be also folded?
    if (!left->is_constant() || !right->is_constant())
        return &be;
    
    auto opk = be.get_op().get_kind();
    // Because of `is_constant` check left and right can be only one of:
    // INT_LITERAL, FLOAT_LITERAL, BOOL_LITERAL, STRING_LITERAL, NIL_LITERAL
    auto lt = left->get_type();
    auto rt = right->get_type();

    // Fold only same types of Int and Float
    if (lt == rt ||
        (lt == IRType::INT_LITERAL && rt == IRType::FLOAT_LITERAL) ||
        (lt == IRType::FLOAT_LITERAL && rt == IRType::INT_LITERAL)) {
        // TODO:
    }

    return &be;
}