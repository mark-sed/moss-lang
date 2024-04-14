/**
 * @file scanner.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Tokenization and lexical analysis of moss code
 */

#ifndef _SCANNER_HPP_
#define _SCANNER_HPP_

#include <string>
#include <utility>

namespace moss {

/** Scanner token types */
enum TokenType {
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_CURLY,     // {
    RIGHT_CURLY,    // }
    LEFT_SQUARE,    // [
    RIGHT_SQUARE,   // ]

    END,        // ; or \n
    COMMA,      // ,
    DOT,        // .
    RANGE,      // ..
    COLON,      // :
    SCOPE,      // ::
    IN_ANNOTATION, // @!
    OUT_ANNOTATION, // @
    NON_LOCAL,  // $
    QUESTION_M, // ?
    BACK_SLASH, // esc
    HASH,       // #
    QUOTE,      // "
    MULT_QUOTE, // """

    CONCAT, // ++
    EXP,    // ^
    PLUS,   // +
    MINUS,  // -
    DIV,    // /
    MUL,    // *
    MOD,    // %

    SET,        // =
    SET_CONCAT, // ++=
    SET_EXP,    // ^=
    SET_PLUS,   // +=
    SET_MINUS,  // -=
    SET_DIV,   // /=
    SET_MUL,    // *=
    SET_MOD,    // %=

    UNWRAP, // <<
    SILENT, // ~

    EQ,  // ==
    NEQ, // !=
    BT,  // >
    LT,  // <
    BEQ, // >=
    LEQ, // <=

    SHORT_C_AND,    // &&
    SHORT_C_OR,     // ||
    AND,            // and
    OR,             // or
    NOT,            // not
    XOR,            // xor
    IN,             // in

    IMPORT,     // import
    AS,         // as
    IF,         // if
    ELSE,       // else
    WHILE,      // while
    DO,         // do
    FOR,        // for
    SWITCH,     // switch
    CASE,       // case
    TRY,        // try
    CATCH,      // catch
    FINALLY,    // finally
    NEW,        // new

    BREAK,      // break
    CONTINUE,   // continue
    RAISE,      // raise
    RETURN,     // return

    ENUM,   // enum
    CLASS,  // class
    SPACE,  // space
    FUN,    // fun

    NIL,    // nil
    TRUE,   // true
    FALSE,  // false
    SUPER,  // super
    THIS,   // this

    INT,        // integer value
    FLOAT,      // floating point value
    STRING,     // string value
    XSTRING,    // xstring value (rstring, fstring, nstring...)

    ID,         // identificator

    END_OF_FILE // end of file or input
    // Not used:
    // !, &, |, `, ', (>>, <<,) 
};


class SourceInfo {
private:
    std::string &filename;
    // start, end
    std::pair<unsigned, unsigned> line;
    // TODO: make this a range
    unsigned col;

public:
    SourceInfo(std::string &filename) : filename(filename) {}

    std::string &get_filename() { return filename; }
};


class Scanner {

};

}

#endif//_SCANNER_HPP_