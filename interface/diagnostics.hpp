/**
 * @file diagnostics.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Diagnostic messages for errors
 */

#ifndef _DIAGNOSTICS_HPP_
#define _DIAGNOSTICS_HPP_

#include "source.hpp"
#include "scanner.hpp"
#include "os_interface.hpp"
#include "utils.hpp"
#include <cassert>

namespace moss {

class Token;
class Scanner;
class ErrorToken;

namespace diags {

enum DiagID {
    UNKNOWN = 0,
    SYNTAX_ERROR,
    DECL_EXPECTED_END,
    ASSERT_MISSING_PARENTH,
    ASSERT_EXPECTS_ARG,
    EXPR_EXPECTED
};

static const char * DIAG_MSGS[] = {
    "Unknown error",
    "Syntax error", // This is for ErrorToken so it will be replaced with custom message
    "Missing new line ('\\n') or semicolon (';') after a declaration",
    "Assert expects its arguments in parenthesis",
    "Assert expects 1 or 2 arguments — condition and optional message",
    "Expecting an expression"
};

class Diagnostic {
public:
    DiagID id;
    const SourceFile &src_f;
    Token *token;
    Scanner *scanner;
    bool warning;
    ustring msg;

    template<typename ... Args>
    Diagnostic(SourceFile &src_f, Token *token, Scanner *scanner, DiagID id, Args ... args) 
               : id(id), src_f(src_f), token(token), scanner(scanner), warning(false) {
        assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
        if (sizeof...(Args) > 0)
            this->msg = utils::formatv(DIAG_MSGS[id], args ...);
        else
            this->msg = DIAG_MSGS[id];
    }

    Diagnostic(ErrorToken *token, Scanner *scanner);
};

}

}

#endif//_DIAGNOSTICS_HPP_