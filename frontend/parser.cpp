#include "parser.hpp"
#include "ir.hpp"
#include "ast.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "errors.hpp"
#include <cassert>

/**
 * This macro asserts that condition is true otherwise it raises a parser_error
 * This macro REQUIRES to be called withing Parser class as it calls its
 * method.
 */ 
#define parser_assert(cond, msg) do { if(!(cond)) parser_error(msg); } while(0)

using namespace moss;
using namespace ir;

IR *Parser::parse(bool is_main) {
    LOG1("Started parsing module");
    Module *m = new Module(this->src_file.get_module_name(), this->src_file, is_main);

    LOG2("Running scanner");
    Token *t = nullptr;
    do {
        t = scanner->next_token();
        tokens.push_back(t);
    } while(t->get_type() != TokenType::END_OF_FILE);
    LOG2("Finished scanning");

    while (!check(TokenType::END_OF_FILE)) {
        IR *decl;
        try {
            decl = declaration();
        } catch (Raise *raise) {
            decl = raise;
            StringLiteral *err_msg = dyn_cast<StringLiteral>(raise->get_exception());
            assert(err_msg && "Error message from parser is not a String literal");
            errs << err_msg->get_value();
            if (is_main) {
                error::exit(error::ErrorCode::RUNTIME_ERROR);
            }
            else {
                return raise;
            }
        }
        assert(decl && "Declaration in parser is nullptr");
        m->push_back(decl);
    }

    // Append EOF, but only when not already present as it may be added by
    // an empty last line
    if (!isa<EndOfFile>(m->back()))
        m->push_back(new ir::EndOfFile());
        
    LOG1("Finished parsing module");
    return m;
}

std::vector<ir::IR *> Parser::parse_line() {
    std::vector<ir::IR *> line_decls;
    tokens.clear();
    curr_token = 0;
    Token *t = nullptr;
    bool padding_start = true;
    do {
        t = scanner->next_token();
        // Skip any whitespace at the start
        if (padding_start && t->get_type() == TokenType::WS)
            continue;
        padding_start = false;
        LOGMAX(*t);
        tokens.push_back(t);
    } while(t->get_type() != TokenType::END_NL && t->get_type() != TokenType::END_OF_FILE);

    while (!check(TokenType::END_NL)) {
        IR *decl;
        try {
            decl = declaration();
        } catch (Raise *raise) {
            decl = raise;
            // Error occurred -- it does not make sense to continue this line
            // as it could have relied on the errorous value, so just append the
            // exception and return it.
            line_decls.push_back(decl);
            return line_decls;
        }
        assert(decl && "Declaration in parser is nullptr");
        line_decls.push_back(decl);
        if (isa<EndOfFile>(decl))
            break;
    }
    return line_decls;
}

void Parser::next_decl() {
    Token *t;
    do {
        t = advance();
    } while (t->get_type() != TokenType::END &&
             t->get_type() != TokenType::END_NL &&
             t->get_type() != TokenType::END_OF_FILE);
    // Advance moves the curr_token to the next one, so no
    // need to advance anymore, we are after the END_X
}

bool Parser::check_ws(TokenType type) {
    return tokens[curr_token]->get_type() == type;
}

bool Parser::match_ws(TokenType type) {
    if (tokens[curr_token]->get_type() == type) {
        advance_ws();
        return true;
    }
    return false;
}

Token *Parser::expect_ws(TokenType type, diags::Diagnostic msg) {
    if (check_ws(type))
        return advance_ws();

    parser_error(msg);
    return nullptr;
}

Token *Parser::advance_ws() {
    if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE ||
        (src_file.get_type() == SourceFile::SourceType::REPL && tokens[curr_token]->get_type() == TokenType::END_NL)) {
        return tokens[curr_token];
    }
    return tokens[curr_token++];
}

bool Parser::check(TokenType type) {
    int offset = 0;
    while (tokens[curr_token+offset]->get_type() == TokenType::WS) {
        ++offset;
    }
    return tokens[curr_token+offset]->get_type() == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token *Parser::expect(TokenType type, diags::Diagnostic msg) {
    if (check(type)) 
        return advance();

    parser_error(msg);
    return nullptr;
}

Token *Parser::advance() {
    while (tokens[curr_token]->get_type() == TokenType::WS) {
        ++curr_token;
    }
    if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE ||
        (src_file.get_type() == SourceFile::SourceType::REPL && tokens[curr_token]->get_type() == TokenType::END_NL)) {
        return tokens[curr_token];
    }
    return tokens[curr_token++];
}

void Parser::parser_error(diags::Diagnostic err_msg) {
    // TODO: Change to specific exception child type (such as TypeError)
    auto str_msg = error::format_error(err_msg);
    throw new Raise(new StringLiteral(str_msg));
}

void Parser::skip_ends() {
    while(match(TokenType::END) || match(TokenType::END_NL))
        ; // Skipping empty new line and ;
}

void Parser::skip_nls() {
    while(match(TokenType::END_NL))
        ; // Skipping empty new line and ;
}

IR *Parser::declaration() {
    LOGMAX("Parsing declaration");
    IR *decl = nullptr;

    // Skip random new lines and ;
    skip_ends();

    // end of file returns End IR
    if (match(TokenType::END_OF_FILE)) {
        return new EndOfFile();
    }

    if (check(TokenType::ERROR_TOKEN)) {
        auto err_tok = dynamic_cast<ErrorToken *>(advance());
        auto str_msg = error::format_error(diags::Diagnostic(err_tok, scanner));
        throw new Raise(new StringLiteral(str_msg));
    }

    // outer / inner annotation

    // import
    // Import has to accept parser errors since it may be in try catch block

    // if

    // while

    // do while

    // for

    // switch

    // try

    // constructor
    
    // assert / raise / return
    if (match(TokenType::ASSERT)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::ASSERT_EXPECTS_ARG));
        // msg can be nullptr
        Expression *msg = nullptr;
        if (match(TokenType::COMMA)) {
            skip_nls();
            msg = expression();
            parser_assert(msg, create_diag(diags::EXPR_EXPECTED));
        }
        expect(TokenType::RIGHT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        decl = new Assert(cond, msg);
    }
    else if (match(TokenType::RAISE)) {
        auto exc = expression();
        parser_assert(exc, create_diag(diags::EXPR_EXPECTED));
        decl = new Raise(exc);
    }
    else if (match(TokenType::RETURN)) { // TODO: Check if inside of a function
        auto exc = expression();
        if (!exc) {
            decl = new Return(new NilLiteral());
        }
        else {
            decl = new Return(exc);
        }
    }
    // break / continue
    else if (match(TokenType::BREAK)) {
        decl = new Break();
    }
    else if (match(TokenType::CONTINUE)) {
        decl = new Continue();
    }
    // enum

    // class

    // space

    // fun

    // expression

    // Every declaration has to end with nl or semicolon or eof
    if(!match(TokenType::END_NL) && !match(TokenType::END) && !check(TokenType::END_OF_FILE)) {
        parser_error(create_diag(diags::EXPECTED_END));
    }
    LOGMAX("Parsed declaration " << *decl);
    return decl;
}

Expression *Parser::expression() {
    if (check(TokenType::ID)) {
        auto id = advance();
        
        if (check(TokenType::LEFT_PAREN)) {
            // Function call
            // TODO
        }
        else {
            return new Variable(id->get_value());
        }
    }
    
    return nullptr;
}

#undef parser_assert