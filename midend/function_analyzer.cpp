#include "function_analyzer.hpp"
#include "parser.hpp"
#include <set>

using namespace moss;
using namespace ir;

void FunctionAnalyzer::check_arguments(const std::vector<ir::Argument *> &args, ustring fname) {
    // Check that argument names differ
    std::set<ustring> existing;
    bool vararg = false;
    for (size_t i = 0; i < args.size(); ++i) {
        auto a = args[i];
        auto name = a->get_name();
        parser_assert(existing.count(name) == 0, parser.create_diag(a->get_src_info(), diags::DUPLICATE_ARG,
            name.c_str(), fname.c_str()));
        parser_assert(!vararg || a->has_default_value(), parser.create_diag(a->get_src_info(), diags::NON_DEFAULT_ARG_AFTER_VARARG,
            name.c_str(), fname.c_str()));
        if (a->is_vararg()) {
            assert(!vararg && "multiple varags (this should be checked in parser)");
            vararg = true;
        }
        
        existing.insert(name);
    }
}

void FunctionAnalyzer::visit(Function &fun) {
    check_arguments(fun.get_args(), fun.get_name());
}

void FunctionAnalyzer::visit(Lambda &lf) {
    check_arguments(lf.get_args(), "lambda");
}