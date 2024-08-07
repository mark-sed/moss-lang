/**
 * @file parser.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Semantical analysis of code and AST generation
 */

#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include "scanner.hpp"
#include "source.hpp"
#include "ast.hpp"
#include "diagnostics.hpp"
#include <memory>
#include <vector>

namespace moss {

/**
 * @brief Moss token parsing into AST
 */
class Parser {
private:
    SourceFile &src_file;
    std::unique_ptr<Scanner> scanner;
    
    size_t curr_token;
    std::vector<Token *> tokens;

    ir::IR *declaration();
    ir::Expression *expression();

    bool check(TokenType type);
    bool match(TokenType type);
    Token *expect(TokenType type, diags::Diagnostic msg);
    Token *advance();

    bool check_ws(TokenType type);
    bool match_ws(TokenType type);
    Token *expect_ws(TokenType type, diags::Diagnostic msg);
    Token *advance_ws();

    inline diags::Diagnostic create_diag(diags::DiagID id) {
        return diags::Diagnostic(id, this->src_file, tokens[curr_token]);
    }
public:
    Parser(SourceFile &file) : src_file(file), scanner(new Scanner(file)), curr_token(0) {}

    ir::Module *parse(bool is_main=false);
};

}

#endif//_PARSER_HPP_