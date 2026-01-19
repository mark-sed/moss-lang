#include "function_analyzer.hpp"
#include "parser.hpp"
#include "builtins.hpp"
#include <set>

using namespace moss;
using namespace ir;

void FunctionAnalyzer::check_arguments(const std::vector<ir::Argument *> &args, ustring fname) {
    // Check that argument names differ
    std::set<ustring> existing;
    bool vararg = false;
    bool def_val_found = false;
    for (size_t i = 0; i < args.size(); ++i) {
        auto a = args[i];
        auto name = a->get_name();
        parser_assert(existing.count(name) == 0, parser.create_diag(a->get_src_info(), diags::DUPLICATE_ARG,
            name.c_str(), fname.c_str()));
        parser_assert(!vararg || a->has_default_value(), parser.create_diag(a->get_src_info(), diags::NON_DEFAULT_ARG_AFTER_VARARG,
            name.c_str(), fname.c_str()));
        parser_assert(!def_val_found || a->has_default_value() || a->is_vararg(), parser.create_diag(a->get_src_info(), diags::NON_DEFAULT_ARG_AFTER_DEFVAL,
            name.c_str(), fname.c_str()));
        if (a->is_vararg()) {
            assert(!vararg && "multiple varags (this should be checked in parser)");
            vararg = true;
        }
        if (a->has_default_value())
            def_val_found = true;
        existing.insert(name);
    }
}

void FunctionAnalyzer::check_annotated_fun(IR &fun, const std::vector<ir::Argument *> &args) {
    if (fun.has_annotation(annots::CONVERTER)) {
        parser_assert(!fun.has_annotation(annots::GENERATOR), parser.create_diag(fun.get_src_info(),
                diags::INCOMPATIBLE_ANNOTS, annots::CONVERTER, annots::GENERATOR, fun.get_name().c_str()));
        parser_assert(args.size() == 1, parser.create_diag(fun.get_src_info(),
                diags::CONVERTER_INCORR_ARGS, fun.get_name().c_str()));
    } else if (fun.has_annotation(annots::GENERATOR)) {
        // No need to check for having converter annotation as it would be checked above.
        parser_assert(args.size() == 1, parser.create_diag(fun.get_src_info(),
                diags::GENERATOR_INCORR_ARGS, fun.get_name().c_str()));
    }
    // Don't use else if as we need to catch incorrect combination of annotations
    if (fun.has_annotation(annots::MAIN)) {
        parser_assert(!fun.get_parent(), parser.create_diag(fun.get_src_info(), diags::NON_GLOBAL_MAIN));
        parser_assert(!has_main, parser.create_diag(fun.get_src_info(), diags::MULTIPLE_MAINS));
        parser_assert(!fun.has_annotation(annots::GENERATOR), parser.create_diag(fun.get_src_info(),
                diags::INCOMPATIBLE_ANNOTS, annots::MAIN, annots::GENERATOR, fun.get_name().c_str()));
        parser_assert(!fun.has_annotation(annots::CONVERTER), parser.create_diag(fun.get_src_info(),
                diags::INCOMPATIBLE_ANNOTS, annots::MAIN, annots::CONVERTER, fun.get_name().c_str()));
        parser_assert(args.size() == 0, parser.create_diag(fun.get_src_info(), diags::MAIN_INCORR_ARGS, fun.get_name().c_str()));
        this->has_main = true;
    }
}

IR *FunctionAnalyzer::visit(Function &fun) {
    check_arguments(fun.get_args(), fun.get_name());
    // Method analyzer was already run and methods are marked, report any
    // operator function that are not methods.
    parser_assert(!Parser::is_operator_fun(fun.get_name()) || fun.is_method(), parser.create_diag(fun.get_src_info(), diags::OP_FUN_OUTSIDE_CLASS, fun.get_name().c_str()));
    parser_assert(!fun.has_annotation(annots::CONVERTER) || !fun.is_method(), parser.create_diag(fun.get_src_info(), diags::DISALLOWED_METHOD_ANNOT, fun.get_name().c_str(), annots::CONVERTER));
    parser_assert(!fun.has_annotation(annots::GENERATOR) || !fun.is_method(), parser.create_diag(fun.get_src_info(), diags::DISALLOWED_METHOD_ANNOT, fun.get_name().c_str(), annots::GENERATOR));
    if (!fun.get_annotations().empty())
        check_annotated_fun(fun, fun.get_args());
    return &fun;
}

IR *FunctionAnalyzer::visit(Return &ret) {
    auto fun = ret.get_outter_ir(IRType::FUNCTION);
    if (!fun)
        fun = ret.get_outter_ir(IRType::LAMBDA);
    parser_assert(fun, parser.create_diag(ret.get_src_info(), diags::RETURN_OUTSIDE_FUN));
    parser_assert(!fun->has_annotation(annots::GENERATOR) || isa<ir::NilLiteral>(ret.get_expr()), parser.create_diag(ret.get_src_info(), diags::RETURN_IN_GENERATOR));
    parser_assert(!fun->has_annotation(annots::MAIN) || isa<ir::NilLiteral>(ret.get_expr()), parser.create_diag(ret.get_src_info(), diags::RETURN_IN_MAIN));
    return &ret;
}

IR *FunctionAnalyzer::visit(Lambda &lf) {
    check_arguments(lf.get_args(), "lambda");
    parser_assert(!Parser::is_operator_fun(lf.get_name()) || lf.is_method(), parser.create_diag(lf.get_src_info(), diags::OP_FUN_OUTSIDE_CLASS, lf.get_name().c_str()));
    auto lambda_name = lf.get_name();
    if (lf.is_anonymous())
        lambda_name = "<anonymous lambda function>";
    parser_assert(!lf.has_annotation(annots::MAIN), parser.create_diag(lf.get_src_info(), diags::LAMBDA_MAIN, lambda_name.c_str()));
    if (!lf.get_annotations().empty())
        check_annotated_fun(lf, lf.get_args());
    return &lf;
}