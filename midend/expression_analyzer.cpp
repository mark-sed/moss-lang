#include "expression_analyzer.hpp"
#include "parser.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

void ExpressionAnalyzer::visit(BinaryExpr &be) {
    auto right = be.get_right();
    auto opk = be.get_op().get_kind();
    if (opk == OperatorKind::OP_ACCESS) {
        auto incorr = !isa<Variable>(right) && !isa<OperatorLiteral>(right) && !isa<SuperLiteral>(right)
                && !isa<AllSymbols>(right);
        if (auto v = dyn_cast<Variable>(right))
            incorr |= v->is_non_local();
        // Appearance of AllSymbol is guarded in parser, that it can be only in import.
        parser_assert(!incorr, parser.create_diag(be.get_src_info(), diags::INCORRECT_ACCESS_SYNATAX));
    }
}

void ExpressionAnalyzer::visit(UnaryExpr &ue) {
    auto opk = ue.get_op().get_kind();
    // FIXME: This is the case only when then expression is not from import, since you can import non-local space
    //if (opk == OperatorKind::OP_SCOPE) {
    //    // TODO: Isn't expr guaranteeed to be a variable?
    //    if (auto v = dyn_cast<Variable>(ue.get_expr())) {
    //        if (!currently_visiting || !isa<Import>(currently_visiting))
    //            parser_assert(!v->is_non_local(), parser.create_diag(ue.get_src_info(), diags::NON_LOCAL_AFTER_GLOBAL));
    //    }
    //}
}

void ExpressionAnalyzer::check_call_arg(Expression *arg) {
    if (auto be = dyn_cast<BinaryExpr>(arg)) {
        if (be->get_op().get_kind() == OperatorKind::OP_SET) {
            // Named argument
            auto *arg_name = be->get_left();
            parser_assert(isa<ir::Variable>(arg_name) || isa<ir::ThisLiteral>(arg_name), parser.create_diag(arg_name->get_src_info(), diags::INCORRECT_ARG_NAME));
        }
    }
}

void ExpressionAnalyzer::visit(Call &c) {
    for (auto a: c.get_args()) {
        check_call_arg(a);
    }
}