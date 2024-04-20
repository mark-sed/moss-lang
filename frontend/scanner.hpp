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
#include <istream>

namespace moss {

/** Scanner token types */
enum class TokenType {
    LEFT_PAREN,     ///< (
    RIGHT_PAREN,    ///< )
    LEFT_CURLY,     ///< {
    RIGHT_CURLY,    ///< }
    LEFT_SQUARE,    ///< [
    RIGHT_SQUARE,   ///< ]

    END,            ///< ;
    END_NL,         ///< \n
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

    END_OF_FILE, ///< end of file or input
    WS,          ///< whitespace
    UNKNOWN      ///< incorrect token
    // Not used:
    // !, &, |, `, ', #, >> 
};


inline std::ostream& operator<< (std::ostream& os, const TokenType tt) {
    switch(tt) {
        case TokenType::LEFT_PAREN: os << "LEFT_PAREN"; break;
        case TokenType::RIGHT_PAREN: os << "RIGHT_PAREN"; break;
        case TokenType::LEFT_CURLY: os << "LEFT_CURLY"; break;
        case TokenType::RIGHT_CURLY: os << "RIGHT_CURLY"; break;
        case TokenType::LEFT_SQUARE: os << "LEFT_SQUARE"; break;
        case TokenType::RIGHT_SQUARE: os << "RIGHT_SQUARE"; break;
        case TokenType::END: os << "END"; break;
        case TokenType::END_NL: os << "END_NL"; break;
        case TokenType::COMMA: os << "COMMA"; break;
        case TokenType::DOT: os << "DOT"; break;
        case TokenType::RANGE: os << "RANGE"; break;
        case TokenType::COLON: os << "COLON"; break;
        case TokenType::SCOPE: os << "SCOPE"; break;
        case TokenType::IN_ANNOTATION: os << "IN_ANNOTATION"; break;
        case TokenType::OUT_ANNOTATION: os << "OUT_ANNOTATION"; break;
        case TokenType::NON_LOCAL: os << "NON_LOCAL"; break;
        case TokenType::QUESTION_M: os << "QUESTION_M"; break;
        case TokenType::BACK_SLASH: os << "BACK_SLASH"; break;
        case TokenType::SL_COMMENT: os << "SL_COMMENT"; break;
        case TokenType::QUOTE: os << "QUOTE"; break;
        case TokenType::MULT_QUOTE: os << "MULT_QUOTE"; break;
        case TokenType::CONCAT: os << "CONCAT"; break;
        case TokenType::EXP: os << "EXP"; break;
        case TokenType::PLUS: os << "PLUS"; break;
        case TokenType::MINUS: os << "MINUS"; break;
        case TokenType::DIV: os << "DIV"; break;
        case TokenType::MUL: os << "MUL"; break;
        case TokenType::MOD: os << "MOD"; break;
        case TokenType::SET: os << "SET"; break;
        case TokenType::SET_CONCAT: os << "SET_CONCAT"; break;
        case TokenType::SET_EXP: os << "SET_EXP"; break;
        case TokenType::SET_PLUS: os << "SET_PLUS"; break;
        case TokenType::SET_MINUS: os << "SET_MINUS"; break;
        case TokenType::SET_DIV: os << "SET_DIV"; break;
        case TokenType::SET_MUL: os << "SET_MUL"; break;
        case TokenType::SET_MOD: os << "SET_MOD"; break;
        case TokenType::UNWRAP: os << "UNWRAP"; break;
        case TokenType::SILENT: os << "SILENT"; break;
        case TokenType::EQ: os << "EQ"; break;
        case TokenType::NEQ: os << "NEQ"; break;
        case TokenType::BT: os << "BT"; break;
        case TokenType::LT: os << "LT"; break;
        case TokenType::BEQ: os << "BEQ"; break;
        case TokenType::LEQ: os << "LEQ"; break;
        case TokenType::SHORT_C_AND: os << "SHORT_C_AND"; break;
        case TokenType::SHORT_C_OR: os << "SHORT_C_OR"; break;
        case TokenType::AND: os << "AND"; break;
        case TokenType::OR: os << "OR"; break;
        case TokenType::NOT: os << "NOT"; break;
        case TokenType::XOR: os << "XOR"; break;
        case TokenType::IN: os << "IN"; break;
        case TokenType::IMPORT: os << "IMPORT"; break;
        case TokenType::AS: os << "AS"; break;
        case TokenType::IF: os << "IF"; break;
        case TokenType::ELSE: os << "ELSE"; break;
        case TokenType::WHILE: os << "WHILE"; break;
        case TokenType::DO: os << "DO"; break;
        case TokenType::FOR: os << "FOR"; break;
        case TokenType::SWITCH: os << "SWITCH"; break;
        case TokenType::CASE: os << "CASE"; break;
        case TokenType::TRY: os << "TRY"; break;
        case TokenType::CATCH: os << "CATCH"; break;
        case TokenType::FINALLY: os << "FINALLY"; break;
        case TokenType::NEW: os << "NEW"; break;
        case TokenType::ASSERT: os << "ASSERT"; break;
        case TokenType::BREAK: os << "BREAK"; break;
        case TokenType::CONTINUE: os << "CONTINUE"; break;
        case TokenType::RAISE: os << "RAISE"; break;
        case TokenType::RETURN: os << "RETURN"; break;
        case TokenType::ENUM: os << "ENUM"; break;
        case TokenType::CLASS: os << "CLASS"; break;
        case TokenType::SPACE: os << "SPACE"; break;
        case TokenType::FUN: os << "FUN"; break;
        case TokenType::NIL: os << "NIL"; break;
        case TokenType::TRUE: os << "TRUE"; break;
        case TokenType::FALSE: os << "FALSE"; break;
        case TokenType::SUPER: os << "SUPER"; break;
        case TokenType::THIS: os << "THIS"; break;
        case TokenType::INT: os << "INT"; break;
        case TokenType::FLOAT: os << "FLOAT"; break;
        case TokenType::STRING: os << "STRING"; break;
        case TokenType::XSTRING: os << "XSTRING"; break;
        case TokenType::ID: os << "ID"; break;
        case TokenType::END_OF_FILE: os << "END_OF_FILE"; break;
        case TokenType::WS: os << "WS"; break;
        case TokenType::UNKNOWN: os << "UNKNOWN"; break;
        default: {
            assert(false && "Missing token type to string");
            os << "MISSING";
        }
    }
    return os;
}

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
    std::istream *get_new_stream();
    std::string get_name() {
        if (type == SourceType::STRING) return "<one-liner>";
        return path_or_code;
    }
};


/** Stores source file information for a given token */
class SourceInfo {
private:
    const SourceFile &file;
    std::pair<unsigned, unsigned> lines; ///< Range of lines of the token
    std::pair<unsigned, unsigned> cols;  ///< Starting column and ending column

public:
    SourceInfo(const SourceFile &file, unsigned line_start, unsigned line_end, unsigned col_start, unsigned col_end) 
        : file(file), lines(std::make_pair(line_end, line_end)), cols(std::make_pair(col_start, col_end)) {}
    SourceInfo(const SourceFile &file, std::pair<unsigned, unsigned> lines, std::pair<unsigned, unsigned> cols) 
        : file(file), lines(lines), cols(cols) {}

    const SourceFile &get_file() { return file; }
};

/** Object represents a scanner token */
class Token {
private:
    std::string value;
    TokenType type;
    SourceInfo src_info;
public:
    Token(std::string value, TokenType type, SourceInfo src_info) : value(value), type(type), src_info(src_info) {}

    std::string get_value() { return this->value; }
    TokenType get_type() { return this->type; }
    SourceInfo get_src_info() { return this->src_info; }

    friend std::ostream& operator<< (std::ostream& os, Token &t) {
        os << "(" << t.type << ")\"" << t.value << "\"";
        return os;
    }
};

class Scanner {
private:
    SourceFile &file;
    std::istream *stream;
    unsigned line;
    unsigned col;
    unsigned len;

    Token *tokenize(std::string value, TokenType type);
    Token *tokenize(int value, TokenType type);

    bool check_and_advance(char c);

    inline int advance() { ++this->len; return stream->get(); }
    inline int peek() { return stream->peek(); }
public:
    Scanner(SourceFile &file) : file(file), line(1), col(1), len(0) {
        this->stream = file.get_new_stream();
    }

    Token *next_token();
    Token *next_nonws_token();
};

}

#endif//_SCANNER_HPP_