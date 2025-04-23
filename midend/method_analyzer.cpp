#include "method_analyzer.hpp"
#include "diagnostics.hpp"
#include "ir_visitor.hpp"
#include "parser.hpp"

using namespace moss;
using namespace ir;

void MethodAnalyzer::check_constructor(Function &method, ustring class_name) {
    if (method.get_name() == class_name) {
        LOGMAX("Setting " << method.get_name() << " as constructor");
        method.set_constructor(true);
    }
}

void MethodAnalyzer::visit(Class &cls) {
    for (auto i : cls.get_body()) {
        if (auto f = dyn_cast<Function>(i)) {
            check_constructor(*f, cls.get_name());
            LOGMAX("Setting " << f->get_name() << " as method");
            f->set_method(true);
        } else if (auto l = dyn_cast<Lambda>(i)) {
            parser_assert(l->get_name() != cls.get_name(), parser.create_diag(l->get_src_info(), diags::LAMBDA_CONSTRUCTOR)); 
            LOGMAX("Setting " << l->get_name() << " as method");
            l->set_method(true);
        }
    }
}

void MethodAnalyzer::visit(Function &fun) {
    this->in_constructor = fun.is_constructor();
}

void MethodAnalyzer::visit(Return &ret) {
    if (in_constructor) {
        auto rval = ret.get_expr();
        parser_assert(!rval || isa<NilLiteral>(rval), parser.create_diag(ret.get_src_info(), diags::NON_NIL_RETURN_IN_CONSTR)); 
    }
}