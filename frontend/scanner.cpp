#include "scanner.hpp"
#include "errors.hpp"
#include <istream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>

using namespace moss;

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
    // FIXME: On Windows this does seem to work correctly
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

void Scanner::unput() {
    this->curr_byte--;
    this->len--;
}

Token *Scanner::tokenize(ustring value, TokenType type) {
    Token *t = new Token(value, type, SourceInfo(file, this->line, this->line, this->col, this->col+this->len));
    this->col += this->len;
    return t;
}

Token *Scanner::tokenize(int value, TokenType type) {
    return tokenize(ustring(1, value), type);
}

template<typename ... Args>
ErrorToken *Scanner::err_tokenize(ustring value, ustring note, error::msgtype msg, Args ... args) {
    ErrorToken *t = new ErrorToken(value, SourceInfo(file, this->line, this->line, this->col, this->col+this->len), note, msg, args ...);
    this->col += this->len;
    return t;
}

template<typename ... Args>
ErrorToken *Scanner::err_tokenize(int value, ustring note, error::msgtype msg, Args ... args) {
    return err_tokenize(ustring(1, value), note, msg, args ...);
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

ustring Scanner::read_incorrect(ustring start) {
    ustring val = start;
    auto next_c = peek();
    while (is_part_of_id(next_c)) {
        val += next_c.to_str();
        advance();
        next_c = peek();
    }
    return val;
}

Token *Scanner::parse_id_or_keyword(int start) {
    return parse_id_or_keyword(ustring(1, start));
}

static bool is_digit(int c, int base) {
    if (base == 10) return std::isdigit(c);
    if (base == 2) return c == '0' || c == '1';
    if (base == 8) return c >= '0' && c < '8';
    if (base == 16) return std::isxdigit(c);
    assert(false && "Unknown base");
    return false;
}

// Returns float or int 
// int is always converted to base 10
// float might be in scientific notation
Token *Scanner::parse_number(int start) {
    ustring number_str(1, start);
    int next_c = peek_nonutf();
    int base = 10;
    // Check if the number is specific base 
    if (start == '0') {
        if (next_c == 'x' || next_c == 'X') {
            base = 16;
            number_str += advance().to_str();
            next_c = peek_nonutf();
        }
        else if (next_c == 'q' || next_c == 'Q') {
            base = 8;
            number_str += advance().to_str();
            next_c = peek_nonutf();
        }
        else if (next_c == 'b' || next_c == 'B') {
            base = 2;
            number_str += advance().to_str();
            next_c = peek_nonutf();
        }
    }
    while (is_digit(next_c, base)) {
        number_str += ustring(1, next_c);
        advance();
        next_c = peek_nonutf();
    }
    char last_dig = number_str[number_str.size()-1];
    bool is_last_dig_base = last_dig == 'x' || last_dig == 'X' || last_dig == 'q' || last_dig == 'Q' || last_dig == 'b' || last_dig == 'B'; 
    if ((is_part_of_id(next_c) || is_last_dig_base) && next_c != 'e' && next_c != 'E') {
        // number followed by character or number that is outside of its base or base was set, but no digits were set
        number_str = read_incorrect(number_str);
        if (base == 2)
            return err_tokenize(number_str, "", error::msgs::INCORRECT_INT_LITERAL, "binary");
        if (base == 8)
            return err_tokenize(number_str, "", error::msgs::INCORRECT_INT_LITERAL, "octal");
        if (base == 16)
            return err_tokenize(number_str, "", error::msgs::INCORRECT_INT_LITERAL, "hexadecimal");
        return err_tokenize(number_str, "", error::msgs::INCORRECT_INT_LITERAL, "decimal");
    }

    if (next_c == '.') {
        // Float parsing
        number_str += advance().to_str();
        // This might be range
        next_c = peek_nonutf();
        if (next_c == '.') {
            // Range "number.."
            number_str.pop_back();
            unput();
        }
        else {
            // Float
            if (base != 10) {
                number_str = read_incorrect(number_str);
                return err_tokenize(number_str, "If you want to access object member use parenthesis", error::msgs::FLOAT_NON_DEC_BASE, "");
            }
            while (std::isdigit(next_c)) {
                number_str += ustring(1, next_c);
                advance();
                next_c = peek_nonutf();
            }
            if (next_c != 'e' && next_c != 'E')
                return tokenize(number_str, TokenType::FLOAT);
        }
    }
    
    if (next_c == 'e' || next_c == 'E') {
        // Float in scientific notation
        number_str += advance().to_str();
        next_c = peek_nonutf();
        bool sign_without_digit = false;
        if (next_c == '-' || next_c == '+') {
            number_str += advance().to_str();
            next_c = peek_nonutf();
            sign_without_digit = true;
        }
        while (std::isdigit(next_c)) {
            sign_without_digit = false;
            number_str += ustring(1, next_c);
            advance();
            next_c = peek_nonutf();
        }
        if (sign_without_digit) {
            number_str = read_incorrect(number_str);
            return err_tokenize(number_str, "Dangling sign for float exponent", error::msgs::INCORRECT_FLOAT_LITERAL, "");
        }
        if (is_part_of_id(next_c)) {
            number_str = read_incorrect(number_str);
            return err_tokenize(number_str, "Float exponent has to be a whole decimal number", error::msgs::INCORRECT_FLOAT_LITERAL, "");
        }
        // Check if exponent has a value after it
        if (number_str[number_str.size()-1] == 'e' || number_str[number_str.size()-1] == 'E') {
            return err_tokenize(number_str, "Missing exponent value", error::msgs::INCORRECT_FLOAT_LITERAL, "");
        }
        // No need to check base since e is hexdigit
        assert(base == 10 && "somehow scientific double is not base 10");
        return tokenize(number_str, TokenType::FLOAT);
    }

    if (base != 10) {
        long num_long = std::stoul(number_str, nullptr, base);
        return tokenize(std::to_string(num_long), TokenType::INT);
    }
    return tokenize(number_str, TokenType::INT);
}

Token *Scanner::parse_multi_comment() {
    auto next_c = advance();
    std::string newlines = "";
    while (next_c.is_utf || next_c.c != EOF) {
        if (!next_c.is_utf && next_c.c == '*') {
            if (peek_nonutf() == '/') {
                advance();
                if (newlines.empty())
                    return tokenize(" ", TokenType::WS);
                else
                    return tokenize(newlines, TokenType::END_NL);
            }
        }
        else if (!next_c.is_utf && next_c.c == '\n') {
            newlines += "\n";
        }
        next_c = advance();
    }
    return err_tokenize("/*", "", error::msgs::UNTERMINATED_COMMENT, "");
}

Token *Scanner::parse_string(bool triple_quote) {
    ustring value = "";
    auto next_c = advance();
    while (next_c.is_utf || next_c.c != EOF) {
        if (!next_c.is_utf && next_c.c == '"') {
            // Check for escaped quote
            if(value.empty() || value.back() != '\\') {
                if (!triple_quote) {
                    return tokenize(value, TokenType::STRING);
                }
                // Check if triple quote
                else if(peek_nonutf() == '"') {
                    // found ""
                    advance();
                    if(peek_nonutf() == '"') {
                        // Last quote in triple quote
                        advance();
                        return tokenize(value, TokenType::STRING);
                    }
                    value += "\"\"";
                    next_c = advance();
                    continue;
                }
            }
        }
        else if (!next_c.is_utf && next_c.c == '\n') {
            this->line++;
            if (!triple_quote) {
                return err_tokenize(value, "", error::msgs::UNTERMINATED_STRING_LITERAL, "");
            }
        }
        value += next_c.to_str();
        next_c = advance();
    }
    return err_tokenize(value, "", error::msgs::UNTERMINATED_STRING_LITERAL, "");
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
        if (std::isspace(c) && c != '\n') {
            ustring space_str(1, c);
            c = peek_nonutf();
            while (std::isspace(c) && c != '\n') {
                space_str.append(1, advance().c);
                c = peek_nonutf();
            }
            return tokenize(space_str, TokenType::WS);
        }
        // ID or keyword
        if (std::isalpha(c) || c == '_') {
            return parse_id_or_keyword(c);
        }
        // int or float
        if (std::isdigit(c)) {
            return parse_number(c);
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
                if (check_and_advance('/')) {
                    auto next_c = advance();
                    while(next_c.is_utf || (next_c.c != '\n' && next_c.c != EOF)) {
                        next_c = advance();
                    }
                    if (next_c.c == EOF)
                        return tokenize("\n", TokenType::END_OF_FILE);
                    return tokenize("\n", TokenType::END_NL);
                }
                if (check_and_advance('*')) {
                    return parse_multi_comment();
                }
                    
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
                return err_tokenize(c, "Negation is done with 'not' or did you mean '!='?", error::msgs::UNKNOWN_SYMBOL, "!");
            }
            case '>': {
                if (check_and_advance('='))
                    return tokenize(">=", TokenType::BEQ);
                return tokenize(c, TokenType::BT);
            }
            case '&': {
                if (check_and_advance('&'))
                    return tokenize("&&", TokenType::SHORT_C_AND);
                return err_tokenize(c, "Bitwise and is done with 'and' or did you mean '&&'?", error::msgs::UNKNOWN_SYMBOL, "&");
            }
            case '|': {
                if (check_and_advance('|'))
                    return tokenize("||", TokenType::SHORT_C_OR);
                return err_tokenize(c, "Bitwise or is done with 'or' or did you mean '||'?", error::msgs::UNKNOWN_SYMBOL, "!");
            }
            case '"': {
                if (check_and_advance('"')) {
                    if(check_and_advance('"')) {
                        // triple quote string
                        return parse_string(true);
                    }
                    // Empty string
                    return tokenize("", TokenType::STRING);
                }
                // Single quote string
                return parse_string(false);
            }
            case EOF: return tokenize(c, TokenType::END_OF_FILE);
            // Error cases with notes
            case '\'': return err_tokenize(c, "Strings can be in single quotes (\"text\") or triple quotes (\"\"\"test\"\"\")", error::msgs::UNKNOWN_SYMBOL, ustring(1, c).c_str());
        }

        // Unknown token
        return err_tokenize(c, "", error::msgs::UNKNOWN_SYMBOL, ustring(1, c).c_str());
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