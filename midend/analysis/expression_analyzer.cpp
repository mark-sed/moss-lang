#include "expression_analyzer.hpp"
#include "parser.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

IR *ExpressionAnalyzer::visit(BinaryExpr &be) {
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
    return &be;
}

IR *ExpressionAnalyzer::visit(UnaryExpr &ue) {
    //auto opk = ue.get_op().get_kind();
    //
    //// Check that scope is not used for path import - it cannot be space
    //if (opk == OperatorKind::OP_SCOPE) {
    //    parser_assert(!(currently_visiting && isa<Import>(currently_visiting) && isa<StringLiteral>(ue.get_expr())), 
    //                    parser.create_diag(ue.get_src_info(), diags::SPACE_IMPORT_PATH));
    //}
    return &ue;
}

void ExpressionAnalyzer::check_call_arg(Expression *arg, std::unordered_set<ustring> &named_args) {
    if (auto be = dyn_cast<BinaryExpr>(arg)) {
        if (be->get_op().get_kind() == OperatorKind::OP_SET) {
            // Named argument
            auto *arg_name = be->get_left();
            parser_assert(isa<ir::Variable>(arg_name) || isa<ir::ThisLiteral>(arg_name), parser.create_diag(arg_name->get_src_info(), diags::INCORRECT_ARG_NAME));
            parser_assert(named_args.count(arg_name->get_name()) == 0, parser.create_diag(arg_name->get_src_info(), diags::DUPLICATE_NAME_IN_CALL, arg_name->get_name().c_str()));
            named_args.insert(arg_name->get_name());
        }
    }
}

IR *ExpressionAnalyzer::visit(Call &c) {
    std::unordered_set<ustring> named_args{};
    for (auto a: c.get_args()) {
        check_call_arg(a, named_args);
    }
    return &c;
}

IR *ExpressionAnalyzer::visit(Break &b) {
    auto loop = b.get_outter_ir({IRType::FOR_LOOP, IRType::WHILE, IRType::DO_WHILE});
    parser_assert(loop, parser.create_diag(b.get_src_info(), diags::BREAK_OUTSIDE_OF_LOOP));
    return &b;
}

IR *ExpressionAnalyzer::visit(Continue &c) {
    auto loop = c.get_outter_ir({IRType::FOR_LOOP, IRType::WHILE, IRType::DO_WHILE});
    parser_assert(loop, parser.create_diag(c.get_src_info(), diags::CONTINUE_OUTSIDE_OF_LOOP));
    return &c;
}

void ExpressionAnalyzer::check_import_expr(Expression *e) {
    if (auto ue = dyn_cast<UnaryExpr>(e)) {
        if (ue->get_op().get_kind() == OperatorKind::OP_SCOPE) {
            parser_assert(!isa<StringLiteral>(ue->get_expr()), parser.create_diag(ue->get_src_info(), diags::SPACE_IMPORT_PATH));
        }
        check_import_expr(ue->get_expr());
    } else if (auto be = dyn_cast<BinaryExpr>(e)) {
        check_import_expr(be->get_left());
        check_import_expr(be->get_right());
    }
}

IR *ExpressionAnalyzer::visit(Import &im) {
    for (auto n: im.get_names()) {
        check_import_expr(n);
    }
    return &im;
}