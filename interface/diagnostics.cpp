#include "diagnostics.hpp"

using namespace moss;
using namespace diags;

/*template<typename ... Args>
Diagnostic::Diagnostic(File &src_f, Token *token, Scanner *scanner, DiagID id, Args ... args) 
            : id(id), src_f(src_f), src_info(token->get_src_info()), scanner(scanner) {
    static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
    assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
    if (sizeof...(Args) > 0)
        this->msg = utils::formatv(DIAG_MSGS[id], args ...);
    else
        this->msg = DIAG_MSGS[id];
}*/

Diagnostic::Diagnostic(ErrorToken *token, Scanner *scanner) 
            : id(DiagID::SYNTAX_ERROR), src_f(token->get_src_info().get_file()), src_info(token->get_src_info()), scanner(scanner) {
    this->msg = token->get_report();
}

/*template<typename ... Args>
Diagnostic::Diagnostic(File &src_f, DiagID id, Args ... args) 
            : id(id), src_f(src_f), src_info(SourceInfo(src_f, 0, 0, 0, 0)), scanner(nullptr) {
    static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
    assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
    if (sizeof...(Args) > 0)
        this->msg = utils::formatv(DIAG_MSGS[id], args ...);
    else
        this->msg = DIAG_MSGS[id];
}*/