/// 
/// \file scanner.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Tokenization and lexical analysis of moss code
/// 

#ifndef _SCANNER_HPP_
#define _SCANNER_HPP_

#include "commons.hpp"
#include "source.hpp"
#include "utils.hpp"
#include "errors.hpp"
#include <string>
#include <fstream>
#include <cassert>
#include <istream>
#include <cstdio>
#include <unordered_map>
#include <list>

namespace moss {

namespace error {
    using msgtype = const char *;
    namespace msgs {
    }
}

/// Scanner token types
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
    THREE_DOTS,     ///< ...
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

    UNPACK, ///< <<
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
    DEFAULT,    ///< default
    TRY,        ///< try
    CATCH,      ///< catch
    FINALLY,    ///< finally
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

    INT,        ///< decimal integer value
    FLOAT,      ///< floating point value
    STRING,     ///< string value
    FSTRING,    ///< fstring value

    ID,         ///< identificator

    END_OF_FILE, ///< end of file or input
    WS,          ///< whitespace
    ERROR_TOKEN  ///< incorrect token (error)
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
        case TokenType::THREE_DOTS: os << "THREE_DOTS"; break;
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
        case TokenType::UNPACK: os << "UNPACK"; break;
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
        case TokenType::FSTRING: os << "FSTRING"; break;
        case TokenType::ID: os << "ID"; break;
        case TokenType::END_OF_FILE: os << "END_OF_FILE"; break;
        case TokenType::WS: os << "WS"; break;
        case TokenType::ERROR_TOKEN: os << "ERROR_TOKEN"; break;
        default: {
            assert(false && "Missing token type to string");
            os << "MISSING";
        }
    }
    return os;
}


/// Object represents a scanner token
class Token {
protected:
    ustring value;
    TokenType type;
    SourceInfo src_info;
public:
    Token(ustring value, TokenType type, SourceInfo src_info) : value(value), type(type), src_info(src_info) {}
    virtual ~Token() {}

    ustring get_value() { return this->value; }
    const char *get_cstr() { return this->value.c_str(); }
    TokenType get_type() { return this->type; }
    SourceInfo get_src_info() { return this->src_info; }

    virtual std::ostream& debug(std::ostream& os) const {
        os << "(" << type << ")\"" << utils::sanitize(value) << "\"";
        return os;
    }
};

inline std::ostream& operator<< (std::ostream& os, Token &t) {
    return t.debug(os);
}

/// Object that represents FString
/// FString is parsed into a list of tokens, where it is string parts with the
/// expression parts
class FStringToken : public Token {
private:
    std::list<Token *> tokens;
public:
    FStringToken(std::list<Token *> tokens, SourceInfo src_info) 
        : Token("<fstring>", TokenType::FSTRING, src_info), tokens(tokens) {}
    ~FStringToken() {}

    std::list<Token *> &get_tokens() { return this->tokens; }
    void push_back(Token *t) { this->tokens.push_back(t); }
};

/// \brief Token that represents syntactic error
/// 
/// Syntactic error has to be accepted and exception has to be raised from it.
/// Because of this there is this special kind of token, which holds the token
/// and message to display with the exception. There is also a note, which
/// might recommend how to fix this error.  
class ErrorToken : public Token {
private:
    ustring note;
    ustring report;
public:
    template<typename ... Args>
    ErrorToken(ustring value, SourceInfo src_info, ustring note, error::msgtype msg, Args ... args)
        : Token(value, TokenType::ERROR_TOKEN, src_info), note(note) {
        report = utils::formatv(msg, args ...);
    }
    ~ErrorToken() {}

    ustring get_report() const {
        if (note.empty())
            return report;
        return report + ". " + note;
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "(" << type << ")[\"" << get_report() << "\"]";
        return os;
    }
};
 
/// \brief Special object that represents a token value
/// 
/// Tokens and values can be a character (int), but also
/// utf-8 character which is better to be saved as string.
/// It would be possible to use only string, but because of speedup
/// it is handled this way. This allows to not create string for
/// one character and also allows to check if value is in fact utf-8
struct UTF8Char {
    int c;
    ustring str;
    bool is_utf;

    UTF8Char(int c) : c(c), str(), is_utf(false) {}
    UTF8Char(ustring str) : c(-1), str(str), is_utf(true) {}

    /// \return this value as string (might or might not be utf-8)
    inline ustring to_str() {
        if (is_utf) return str;
        return std::string(1, c);
    }
};

/// \brief Syntactic analyzer and tokenizer for moss language
/// 
/// Scanner works always with one file/module, this might be input file passed
/// by the user or just string of moss code or stdin. Scanner does not care for
/// this as this is handled by SourceFile class that provides the stream to
/// read from.
class Scanner {
private:
    SourceFile &file;
    std::istream *stream;
    unsigned line;
    unsigned col;
    unsigned len;
    std::vector<ustring> src_text;
    static const std::unordered_map<ustring, TokenType> KEYWORDS;

    Token *tokenize(ustring value, TokenType type);
    Token *tokenize(int value, TokenType type);
    Token *tokenize_fstring(std::list<Token *> tokens);
    template<typename ... Args>
    ErrorToken *err_tokenize(ustring value, ustring note, error::msgtype msg, Args ... args);
    template<typename ... Args>
    ErrorToken *err_tokenize(int value, ustring note, error::msgtype msg, Args ... args);

    bool check_and_advance(char c);
    ustring read_incorrect(ustring start);

    Token *parse_id_or_keyword(ustring start);
    Token *parse_id_or_keyword(int start);
    Token *parse_number(int start);
    Token *parse_string(bool triple_quote, bool fstring=false);
    Token *parse_multi_comment();
    Token *parse_comment_or_shebang();

    ustring curr_line;
    unsigned curr_byte;
    UTF8Char advance();
    UTF8Char peek();
    int peek_nonutf();
    void unput();
public:
    Scanner(SourceFile &file) : file(file), line(0), col(0), len(0), src_text(), curr_line(), curr_byte(0) {
        this->stream = file.get_new_stream();
    }
    ~Scanner() {
        // Only file stream and string stream are allocated
        if (file.get_type() == SourceFile::SourceType::FILE || file.get_type() == SourceFile::SourceType::STRING) {
            delete this->stream;
        }
    }

    /// \return Next token from the current file, including whitespace token
    Token *next_token();
    /// \return Next token from the current file, that is not a whitespace token
    Token *next_nonws_token();

    /// \return Vector that contains lines from the source file (for error reporting)
    std::vector<ustring> get_src_text() { return this->src_text; }
};

}

#endif//_SCANNER_HPP_