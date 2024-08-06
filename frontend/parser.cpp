#include "parser.hpp"
#include "ir.hpp"
#include "ast.hpp"

using namespace moss;
using namespace ir;

Module *Parser::parse(bool is_main) {
    Module *m = new Module(this->src_file.get_module_name(), this->src_file, is_main);

    Token *t = nullptr;
    do {
        t = scanner->next_token();
        tokens.push_back(t);
    } while(t->get_type() != TokenType::END_OF_FILE);

    while (tokens[curr_token]->get_type() != TokenType::END_OF_FILE) {
        m->push_back(parse_declaration());
    }
    return m;
}

bool Parser::check(TokenType type) {
    return tokens[curr_token]->get_type() == type;
}

bool Parser::match(TokenType type) {
    if (tokens[curr_token]->get_type() == type) {
        advance();
        return true;
    }
    return false;
}

Token *Parser::expect(TokenType type, ustring msg) {
    if (check(type)) return advance();

    // TODO: Throw an error
    return nullptr;
}

Token *Parser::advance() {
    return tokens[curr_token++];
}

bool Parser::check_nonws(TokenType type) {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        return tokens[curr_token+1]->get_type() == type;
    }
    return tokens[curr_token]->get_type() == type;
}

bool Parser::match_nonws(TokenType type) {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        advance();
    }
    if (tokens[curr_token]->get_type() == type) {
        advance();
        return true;
    }
    return false;
}

Token *Parser::expect_nonws(TokenType type, ustring msg) {
    if (check_nonws(type)) return advance_nonws();

    // TODO: Throw an error
    return nullptr;
}

Token *Parser::advance_nonws() {
    if (tokens[curr_token]->get_type() == TokenType::WS) {
        ++curr_token;
    }
    return tokens[curr_token++];
}

/*void Parser::skip_ws() {
    while (match(TokenType::WS)) {
        // Consume whitespaces
    }
}*/

IR *Parser::parse_declaration() {
    IR *decl = nullptr;

    // TODO: Assignment
    // Check with if (id()), which checks for id, scope or attrib

    curr_token++;
    return decl;
}

