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
#include <fstream>
#include <cassert>

namespace moss {

/** Scanner token types */
enum TokenType {
    LEFT_PAREN,     ///< (
    RIGHT_PAREN,    ///< )
    LEFT_CURLY,     ///< {
    RIGHT_CURLY,    ///< }
    LEFT_SQUARE,    ///< [
    RIGHT_SQUARE,   ///< ]

    END,            ///< ; or \n
    COMMA,          ///< ,
    DOT,            ///< `.`
    RANGE,          ///< ..
    COLON,          ///< :
    SCOPE,          ///< ::
    IN_ANNOTATION,  ///< @!
    OUT_ANNOTATION, ///< @
    NON_LOCAL,      ///< $
    QUESTION_M,     ///< ?
    BACK_SLASH,     ///< esc
    SL_COMMENT,     ///< //
    QUOTE,          ///< "
    MULT_QUOTE,     ///< """

    CONCAT, ///< ++
    EXP,    ///< ^
    PLUS,   ///< `+`
    MINUS,  ///< `-`
    DIV,    ///< /
    MUL,    ///< `*`
    MOD,    ///< %

    SET,        ///< =
    SET_CONCAT, ///< ++=
    SET_EXP,    ///< ^=
    SET_PLUS,   ///< +=
    SET_MINUS,  ///< -=
    SET_DIV,    ///< /=
    SET_MUL,    ///< *=
    SET_MOD,    ///< %=

    UNWRAP, ///< <<
    SILENT, ///< ~

    EQ,  ///< ==
    NEQ, ///< !=
    BT,  ///< `>`
    LT,  ///< <
    BEQ, ///< >=
    LEQ, ///< <=

    SHORT_C_AND,    ///< &&
    SHORT_C_OR,     ///< ||
    AND,            ///< and
    OR,             ///< or
    NOT,            ///< not
    XOR,            ///< xor
    IN,             ///< in

    IMPORT,     ///< import
    AS,         ///< as
    IF,         ///< if
    ELSE,       ///< else
    WHILE,      ///< while
    DO,         ///< do
    FOR,        ///< for
    SWITCH,     ///< switch
    CASE,       ///< case
    TRY,        ///< try
    CATCH,      ///< catch
    FINALLY,    ///< finally
    NEW,        ///< new
    ASSERT,     ///< assert

    BREAK,      ///< break
    CONTINUE,   ///< continue
    RAISE,      ///< raise
    RETURN,     ///< return

    ENUM,   ///< enum
    CLASS,  ///< class
    SPACE,  ///< space
    FUN,    ///< fun

    NIL,    ///< nil
    TRUE,   ///< true
    FALSE,  ///< false
    SUPER,  ///< super
    THIS,   ///< this

    INT,        ///< integer value
    FLOAT,      ///< floating point value
    STRING,     ///< string value
    XSTRING,    ///< xstring value (rstring, fstring, nstring...)

    ID,         ///< identificator

    END_OF_FILE ///< end of file or input
    // Not used:
    // !, &, |, `, ', (>>, <<,) 
};


/** 
 * Holds information about source "file".
 * It does not have to be physical file, it can be a string of code or stdin.
 */
class SourceFile {
public:
    /** Source File type (file, string of code or standard input) */
    enum class SourceType {
        FILE,
        STRING,
        STDIN,
        INTERACTIVE
    };
private:
    std::string path_or_code;
    SourceType type;
public:
    SourceFile(std::string path_or_code, SourceType src_type) : path_or_code(path_or_code), type(src_type) {
        assert(src_type != SourceType::STDIN && "Path set, but type is STDIN");
        assert(src_type != SourceType::INTERACTIVE && "Path set, but type is INTERACTIVE");
    }
    SourceFile(SourceType src_type) : type(src_type) {
        assert((src_type == SourceType::STDIN || src_type == SourceType::INTERACTIVE) && "File without path");
        if(src_type == SourceType::INTERACTIVE)
            this->path_or_code = "<im>";
        else
            this->path_or_code = "<stdin>";
    }

    std::string get_path_or_code() { return this->path_or_code; }
    SourceType get_type() { return this->type; }
};


/** Stores source file information for a given token */
class SourceInfo {
private:
    const SourceFile &file;
    // start, end
    std::pair<unsigned, unsigned> line;
    // TODO: make this a range
    unsigned col;

public:
    SourceInfo(const SourceFile &file) : file(file) {}

    const SourceFile &get_file() { return file; }
};


class Scanner {
private:
    SourceFile &file;
public:
    Scanner(SourceFile &file) : file(file) {}


};

}

#endif//_SCANNER_HPP_