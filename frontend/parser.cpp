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
        IR *decl;
        try {
            decl = declaration();
        } catch (IR *raise) {
            decl = raise;
            // Eat tokens until next declaration, to recover in interactive mode
            next_decl();
        }
        assert(decl && "Declaration in parser is nullptr");
        m->push_back(decl);
    }
    LOGMAX("Finished parsing module");
    return m;
}

void Parser::next_decl() {
    Token *t;
    do {
        t = advance();
    } while (t->get_type() != TokenType::END &&
             t->get_type() != TokenType::END_NL &&
             t->get_type() != TokenType::END_OF_FILE);
    // Skip also the end of the declaration
    advance();
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

    create_exception(msg);
    return nullptr;
}

Token *Parser::advance_ws() {
    if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE) {
        tokens[curr_token];
    }
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
    if (check(type)) 
        return advance();

    create_exception(msg);
    return nullptr;
}

Token *Parser::advance() {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        ++curr_token;
    }
    else if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE) {
        tokens[curr_token];
    }
    return tokens[curr_token++];
}

Raise *Parser::create_exception(diags::Diagnostic err_msg) {
    if (src_file.get_type() != SourceFile::SourceType::INTERACTIVE) {
        error::error(err_msg);
    }
    // TODO: Change to specific exception child type (such as TypeError)
    auto str_msg = error::format_error(err_msg);
    throw new Raise(new StringLiteral(str_msg));
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

        // TODO: Add raise and change error::error for a raise IR with the error message as an argument

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
    else if (match(TokenType::RAISE)) {
        auto exc = expression();
        if (!exc) {
            assert("Expception is null");
            // Raise error
        }
        decl = new Raise(exc);
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
