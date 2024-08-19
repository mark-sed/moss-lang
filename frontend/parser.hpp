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
#include <vector>

namespace moss {

/**
 * @brief Moss token parsing into AST
 * Parser can parse the whole file at once or go line by line.
 */
class Parser {
private:
    SourceFile &src_file;
    Scanner *scanner;
    
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

    void next_decl();
    void skip_ends();
    void skip_nls();

    template<typename ... Args>
    inline diags::Diagnostic create_diag(diags::DiagID id, Args ... args) {
        return diags::Diagnostic(this->src_file, tokens[curr_token], scanner, id, args ...);
    }

    void parser_error(diags::Diagnostic err_msg);
public:
    Parser(SourceFile &file) : src_file(file), scanner(new Scanner(file)), curr_token(0) {}
    ~Parser() {
        delete scanner;
        for (auto *t: tokens)
            delete t;
    }

    /**
     * Parses a moss file into an AST
     * @return File parsed into a ir::Module. If there is an error then ir::Raise is thrown
     * @throw ir::Raise with parser/scanner error to be rethrown by moss
     */
    ir::IR *parse(bool is_main=false);

    /**
     * Parses a single line or multiple ones if it is a multi-line declaration into an AST
     * @return Vector of parsed IRs. If there is an error then ir::Raise is thrown
     * @throw ir::Raise with parser/scanner error to be rethrown by moss
     */ 
    std::vector<ir::IR *> parse_line();
};

}

#endif//_PARSER_HPP_