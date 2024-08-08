#include "parser.hpp"
#include "ir.hpp"
#include "ast.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "errors.hpp"
#include <cassert>

using namespace moss;
using namespace ir;

Module *Parser::parse(bool is_main) {
    LOGMAX("Started parsing module");
    Module *m = new Module(this->src_file.get_module_name(), this->src_file, is_main);

    LOGMAX("Running scanner");
    Token *t = nullptr;
    do {
        t = scanner->next_token();
        tokens.push_back(t);
        //LOGMAX(*t << " ");
    } while(t->get_type() != TokenType::END_OF_FILE);
    LOGMAX("Finished scanning");

    while (!check(TokenType::END_OF_FILE)) {
        auto decl = declaration();
        assert(decl && "Declaration in parser is nullptr");
        m->push_back(decl);
    }
    LOGMAX("Finished parsing module");
    return m;
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
    if (check_ws(type)) return advance_ws();

    // TODO: Throw an error
    return nullptr;
}

Token *Parser::advance_ws() {
    return tokens[curr_token++];
}

bool Parser::check(TokenType type) {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        return tokens[curr_token+1]->get_type() == type;
    }
    return tokens[curr_token]->get_type() == type;
}

bool Parser::match(TokenType type) {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        advance();
    }
    if (tokens[curr_token]->get_type() == type) {
        advance();
        return true;
    }
    return false;
}

Token *Parser::expect(TokenType type, diags::Diagnostic msg) {
    if (check(type)) return advance();

    error::error(msg);
    return nullptr;
}

Token *Parser::advance() {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        ++curr_token;
    }
    return tokens[curr_token++];
}

IR *Parser::declaration() {
    IR *decl = nullptr;

    // outer / inner annotation

    // import

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
        if (!cond) {
            error::error(create_diag(diags::ASSERT_EXPECTS_ARG));
        }
        // msg can be nullptr
        Expression *msg = nullptr;
        if (match(TokenType::COMMA)) {
            msg = expression();
            if (!msg) {
                // FIXME:
                assert(false && "FIX: Error expected expression");
            }
        }
        expect(TokenType::RIGHT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        decl = new Assert(cond, msg);
    }

    // break / continue

    // enum

    // class

    // space

    // fun

    // expression

    // Every declaration has to end with nl or semicolon or eof
    if(!match(TokenType::END_NL) && !match(TokenType::END) && !check(TokenType::END_OF_FILE)) {
        error::error(create_diag(diags::DECL_EXPECTED_END));
    }
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
