#include "function_analyzer.hpp"
#include "parser.hpp"
#include <set>

using namespace moss;
using namespace ir;

void FunctionAnalyzer::check_arguments(Function &fun) {
    auto args = fun.get_args();
    // Check that argument names differ
    std::set<ustring> existing;
    for (size_t i = 0; i < args.size(); ++i) {
        auto a = args[i];
        auto name = a->get_name();
        parser_assert(existing.count(name) == 0, parser.create_diag(a->get_src_info(), diags::DUPLICATE_ARG,
            name.c_str(), fun.get_name().c_str()));
        existing.insert(name);
    }
}

void FunctionAnalyzer::visit(Function &fun) {
    check_arguments(fun);
}