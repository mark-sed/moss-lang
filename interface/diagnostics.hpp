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

namespace moss {

class Token;

namespace diags {

enum DiagID {
    UNKNOWN = 0,
    DECL_EXPECTED_END,
    ASSERT_MISSING_PARENTH,
    ASSERT_EXPECTS_ARG
};

static const char * DIAG_MSGS[] = {
    "unknown",
    "missing new line ('\\n') or semicolon (';') after a declaration",
    "assert expects its arguments in parenthesis",
    "assert expects 1 or 2 arguments -- condition and optional message"
};

class Diagnostic {
public:
    DiagID id;
    SourceFile &src_f;
    Token *token;
    // TODO: Add params for the msg
    bool warning;

    Diagnostic(DiagID id, SourceFile &src_f, Token *token, bool warning=false) 
               : id(id), src_f(src_f), token(token), warning(warning) {}
};

}

}

#endif//_DIAGNOSTICS_HPP_