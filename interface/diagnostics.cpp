#include "diagnostics.hpp"

using namespace moss;
using namespace diags;

Diagnostic::Diagnostic(ErrorToken *token, Scanner *scanner) 
            : id(DiagID::SYNTAX_ERROR), src_f(token->get_src_info().get_file()), token(token), scanner(scanner) {
    this->msg = token->get_report();
}