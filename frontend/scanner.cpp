#include "scanner.hpp"
#include "errors.hpp"
#include <istream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>

using namespace moss;

std::istream *SourceFile::get_new_stream() {
    switch(this->type) {
        case SourceFile::SourceType::FILE: {
            std::ifstream *f = new std::ifstream(this->path_or_code);
            if(f->fail()){
                error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
            }
            return f;
        }
        case SourceFile::SourceType::STRING: {
            return new std::istringstream(this->path_or_code);
        }
        case SourceFile::SourceType::STDIN: return &std::cin;
        case SourceFile::SourceType::INTERACTIVE: return &std::cin; // FIXME: this should be reading input
    }
    error::error(error::ErrorCode::INTERNAL, "Unknown input format", this);
    return nullptr;
}

UTF8Char Scanner::advance() {
    if (this->curr_byte >= curr_line.size()) {
        std::string l;
        if (!std::getline(*stream, l)) {
            ++this->len;
            return EOF;
        }
        curr_line.assign(l.begin(), l.end());
        ++curr_byte = 0;
        if(stream->peek() != EOF)
            curr_line += '\n';
    }

    int c = curr_line[curr_byte];
    // Read bytes that are continuation of utf-8
    int substr_len = 1;
    while (curr_byte < curr_line.size()) {
        int cn = curr_line[curr_byte+1];
        if ((cn & 0b11000000) == 0b10000000) {
            ++substr_len;
            ++curr_byte;
        }
        else {
            break;
        }
    }
    if (substr_len > 1) {
        ++curr_byte;
        ++this->len;
        std::string utf8 = curr_line.substr(curr_byte-substr_len, substr_len);
        return UTF8Char(utf8);
    }
    ++curr_byte;
    ++this->len;
    return UTF8Char(c);
}

UTF8Char Scanner::peek() {
    // Peek should never happen after a new line, but might on last line
    // Which means its eof
    if (this->curr_byte >= curr_line.size()) {
        assert(stream->peek() == EOF && "Peeked when line was fully read and not on EOF");
        return UTF8Char(EOF);
    }

    auto prev_byte = this->curr_byte;
    auto prev_len = this->len;

    auto c = advance();

    // Restore indexing, since we only peeked
    this->curr_byte = prev_byte;
    this->len = prev_len;

    return UTF8Char(c);
}

int Scanner::peek_nonutf() {
    UTF8Char p = peek();
    if (p.is_utf)
        return 0;
    return p.c;
}

Token *Scanner::tokenize(ustring value, TokenType type) {
    Token *t = new Token(value, type, SourceInfo(file, this->line, this->line, this->col, this->col+this->len));
    this->col += this->len;
    return t;
}

Token *Scanner::tokenize(int value, TokenType type) {
    return tokenize(ustring(1, value), type);
}

bool Scanner::check_and_advance(char c) {
    if(peek_nonutf() == c) {
        advance();
        return true;
    }
    return false;
}

// UTF8 char or _ or 0-9 or letter
static bool is_part_of_id(UTF8Char c) {
    return c.is_utf || c.c == '_' || std::isalnum(c.c);
}

Token *Scanner::parse_id_or_keyword(ustring start) {
    ustring id_str = start;
    auto next_c = peek();
    while (is_part_of_id(next_c)) {
        id_str += next_c.to_str();
        advance();
        next_c = peek();
    }

    // Check if id_str matches any keyword
    if (this->KEYWORDS.find(id_str) != this->KEYWORDS.end()) {
        return tokenize(id_str, KEYWORDS.at(id_str));
    }

    return tokenize(id_str, TokenType::ID);
}

Token *Scanner::parse_id_or_keyword(int start) {
    return parse_id_or_keyword(ustring(1, start));
}

Token *Scanner::next_nonws_token() {
    auto t = next_token();
    // WS is grouped it is not possible to have 2 ws after each other
    if (t->get_type() == TokenType::WS) {
        delete t;
        return next_token();
    }
    return t;
}

Token *Scanner::next_token() {
    this->len = 0;

    UTF8Char utf8c = this->advance();
    if(utf8c.is_utf) {
        ustring c = utf8c.to_str();
        // utf8 can be only identificator
        return parse_id_or_keyword(c);
    }
    else {
        int c = utf8c.c;

        // Consume all whitespace as one
        if(std::isspace(c) && c != '\n') {
            ustring space_str(1, c);
            c = peek_nonutf();
            while(std::isspace(c) && c != '\n') {
                space_str.append(1, advance().c);
                c = peek_nonutf();
            }
            return tokenize(space_str, TokenType::WS);
        }
        // ID or keyword
        if(std::isalnum(c) || c == '_') {
            return parse_id_or_keyword(c);
        }

        switch (c) {
            case '(': return tokenize(c, TokenType::LEFT_PAREN);
            case ')': return tokenize(c, TokenType::RIGHT_PAREN);
            case '{': return tokenize(c, TokenType::LEFT_CURLY);
            case '}': return tokenize(c, TokenType::RIGHT_CURLY);
            case '[': return tokenize(c, TokenType::LEFT_SQUARE);
            case ']': return tokenize(c, TokenType::RIGHT_SQUARE);
            case ';': return tokenize(c, TokenType::END);
            case '\n': {
                auto t = tokenize(c, TokenType::END_NL);
                ++this->line;
                this->col = 0;
                return t;
            }
            case ',': return tokenize(c, TokenType::COMMA);
            case '.': {
                // ".." or  "."
                if (check_and_advance('.'))
                    return tokenize("..", TokenType::RANGE);
                return tokenize(c, TokenType::DOT);
            }
            case ':': {
                // "::" or ":"
                if (check_and_advance(':'))
                    return tokenize("::", TokenType::SCOPE);
                return tokenize(c, TokenType::COLON);
            }
            case '@': {
                if (check_and_advance('!'))
                    return tokenize("@!", TokenType::IN_ANNOTATION);
                return tokenize(c, TokenType::OUT_ANNOTATION);
            }
            case '$': return tokenize(c, TokenType::NON_LOCAL);
            case '?': return tokenize(c, TokenType::QUESTION_M);
            case '\\': return tokenize(c, TokenType::BACK_SLASH);
            //case '\"': return parse_string(); 
            case '+': {
                if (check_and_advance('+')) {
                    // ++ or ++=
                    if (check_and_advance('=')) {
                        return tokenize("++=", TokenType::SET_CONCAT);
                    }
                    return tokenize("++", TokenType::CONCAT);
                }
                // + or  +=
                if (check_and_advance('='))
                    return tokenize("+=", TokenType::SET_PLUS);
                return tokenize(c, TokenType::PLUS);
            }
            case '^': {
                if (check_and_advance('='))
                    return tokenize("^=", TokenType::SET_EXP);
                return tokenize(c, TokenType::EXP);
            }
            case '-': {
                if (check_and_advance('='))
                    return tokenize("-=", TokenType::SET_MINUS);
                return tokenize(c, TokenType::MINUS);
            }
            case '/': {
                if (check_and_advance('='))
                    return tokenize("/=", TokenType::SET_DIV);
                return tokenize(c, TokenType::DIV);
            }
            case '*': {
                if (check_and_advance('='))
                    return tokenize("*=", TokenType::SET_MUL);
                return tokenize(c, TokenType::MUL);
            }
            case '%': {
                if (check_and_advance('='))
                    return tokenize("%=", TokenType::SET_MOD);
                return tokenize(c, TokenType::MOD);
            }
            case '~': return tokenize(c, TokenType::SILENT);
            case '<': {
                if (check_and_advance('<'))
                    return tokenize("<<", TokenType::UNWRAP);
                if (check_and_advance('='))
                    return tokenize("<=", TokenType::LEQ);
                return tokenize(c, TokenType::LT);
            }
            case '=': {
                if (check_and_advance('='))
                    return tokenize("==", TokenType::EQ);
                return tokenize(c, TokenType::SET);
            }
            case '!': {
                if (check_and_advance('='))
                    return tokenize("!=", TokenType::NEQ);
                // ! is not in the language
                // TODO: Recommend "did you mean !=?" or "expected"
                return tokenize(c, TokenType::UNKNOWN);
            }
            case '>': {
                if (check_and_advance('='))
                    return tokenize(">=", TokenType::BEQ);
                return tokenize(c, TokenType::BT);
            }
            case '&': {
                if (check_and_advance('&'))
                    return tokenize("&&", TokenType::SHORT_C_AND);
                // TODO: Recommend "did you mean &&" or "expected"
                return tokenize(c, TokenType::UNKNOWN);
            }
            case '|': {
                if (check_and_advance('|'))
                    return tokenize("||", TokenType::SHORT_C_OR);
                // TODO: Recommend "did you mean ||" or "expected"
                return tokenize(c, TokenType::UNKNOWN);
            }

            //case '': return tokenize(c, TokenType::);
            case EOF: return tokenize(c, TokenType::END_OF_FILE);
        }

        // Unknown token
        return tokenize(c, TokenType::UNKNOWN);
    }
}

const std::unordered_map<ustring, TokenType> Scanner::KEYWORDS = {
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"xor", TokenType::XOR},
    {"in", TokenType::IN},
    
    {"import", TokenType::IMPORT},
    {"as", TokenType::AS},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"do", TokenType::DO},
    {"for", TokenType::FOR},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"new", TokenType::NEW},
    {"assert", TokenType::ASSERT},

    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"raise", TokenType::RAISE},
    {"return", TokenType::RETURN},

    {"enum", TokenType::ENUM},
    {"class", TokenType::CLASS},
    {"space", TokenType::SPACE},
    {"fun", TokenType::FUN},
    
    {"nil", TokenType::NIL},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS}
};