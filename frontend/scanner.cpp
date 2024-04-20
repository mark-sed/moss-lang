#include "scanner.hpp"
#include "errors.hpp"
#include <istream>
#include <iostream>
#include <cstring>
#include <sstream>

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

Token *Scanner::tokenize(std::string value, TokenType type) {
    Token *t = new Token(value, type, SourceInfo(file, this->line, this->line, this->col, this->col+this->len));
    this->col += this->len;
    return t;
}

Token *Scanner::tokenize(int value, TokenType type) {
    return tokenize(std::string(1, value), type);
}

bool Scanner::check_and_advance(char c) {
    if(peek() == c) {
        advance();
        return true;
    }
    return false;
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

    int c = this->advance();
    // Consume all whitespace as one
    if(std::isspace(c) && c != '\n') {
        std::string space_str(1, c);
        c = peek();
        while(std::isspace(c) && c != '\n') {
            space_str.append(1, advance());
            c = peek();
        }
        return tokenize(space_str, TokenType::WS);
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
