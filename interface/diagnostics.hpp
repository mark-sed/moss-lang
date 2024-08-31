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

/** Diagnostic messages and its resources */
namespace diags {

/**
 * @brief ID of diagnostic error
 * This value corresponds to the error message tied to this error type
 */
enum DiagID {
    UNKNOWN = 0,            ///< This value should not be reported
    SYNTAX_ERROR,           ///< Error with syntax (in scanner)
    EXPECTED_END,           ///< Missing ; or nl 
    ASSERT_MISSING_PARENTH, ///< Assert specific - missing () for when someone uses raise syntax
    ASSERT_EXPECTS_ARG,     ///< Assert specific - incorrect arguments
    EXPR_EXPECTED,          ///< Parser expected an expression
    EXPR_EXPECTED_NOTE,     ///< As above but with extra note
    MISSING_RIGHT_PAREN,    ///< No ')' found
    CHAINED_COMPOUND_ASSIGN,    ///< Chained compound assignment operators
    TERNARY_IF_MISSING_FALSE,   ///< Missing false branch in ternary if

    NUMBER_OF_IDS           ///< This value should not be reported it can be used to get the amount of IDs
};

/** 
 * This array holds messages (formatting strings) corresponding to DiagIDs.
 * @note There has to be an entry for every value in DiagsIDs, but the last one
 *       used for getting the enum size.
 */
static const char * DIAG_MSGS[] = {
    "Unknown error",
    "Syntax error", // This is for ErrorToken so it will be replaced with custom message
    "Missing a new line ('\\n') or a semicolon (';') after a statement",
    "Assert expects its arguments in parenthesis",
    "Assert expects 1 or 2 arguments — condition and optional message",
    "Expecting an expression",
    "Expecting an expression — %s",
    "Missing ')'",
    "Compound assignment operators ('%s') cannot be chained",
    "Missing ':' — ternary if requires false branch",
};

/**
 * @brief Diagnostic message for error reporting
 * This class holds all resources needed to provide detailed error report to the
 * user.
 */
class Diagnostic {
public:
    DiagID id;
    const SourceFile &src_f;
    Token *token;
    Scanner *scanner;
    ustring msg;

    template<typename ... Args>
    Diagnostic(SourceFile &src_f, Token *token, Scanner *scanner, DiagID id, Args ... args) 
               : id(id), src_f(src_f), token(token), scanner(scanner) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
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