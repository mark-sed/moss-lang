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
#include "ir.hpp"
#include "source.hpp"
#include "diagnostics.hpp"
#include <vector>
#include <list>
#include <initializer_list>

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

    bool lower_range_prec;
    bool reading_by_lines;
    int multi_line_parsing;

    /** 
     * Parses an IR that is a declaration (if not throws an error)
     * Declaration is a statement that can be on its own, so like a line in REPL
     * 
     * @note This will ignore all standalone new lines and semicolons
     * @throw If error is encountered, this throws ir::Raise describing this
     *        error, which can be interpreted by moss
     * @return Parsed input into an IR class
     */
    ir::IR *declaration();

    /** 
     * Tries to parse an expression
     * 
     * @throw If error is encountered, this throws ir::Raise describing this
     *        error, which can be interpreted by moss
     * @return Parsed input into an Expression class or nullptr if there is
     *         not an expression on the input
    */
    ir::Expression *expression(bool allow_set=false);

    /** Tries to parse a block of code */
    std::list<ir::IR *> block();
    /** 
     * Tries to parse a body,
     * which might be a block of code or a declaration
     */
    std::list<ir::IR *> body();

    ir::Expression *constant();
    std::vector<ir::Expression *> expr_list(bool only_scope_or_id=false, bool allow_set=false);
    std::vector<ir::Argument *> arg_list();
    ir::Argument *argument(bool allow_default_value=false);
    std::list<ir::IR *> cases();

    ir::Expression *unpack();
    ir::Expression *silent();
    ir::Expression *assignment();
    ir::Expression *ternary_if();
    ir::Expression *short_circuit();
    ir::Expression *and_or_xor();
    ir::Expression *op_not();
    ir::Expression *eq_neq();
    ir::Expression *compare_gl();
    ir::Expression *membership();
    ir::Expression *range();
    ir::Expression *concatenation();
    ir::Expression *add_sub();
    ir::Expression *mul_div_mod();
    ir::Expression *exponentiation();
    ir::Expression *unary_plus_minus();
    ir::Expression *element_access();
    ir::Expression *subscript();
    ir::Expression *call();
    ir::Expression *note();
    ir::Expression *scope();

    bool check(TokenType type);
    bool check(std::initializer_list<TokenType> types);
    bool match(TokenType type);
    Token *expect(TokenType type, diags::Diagnostic msg);
    Token *advance();

    bool check_ws(TokenType type);
    bool match_ws(TokenType type);
    Token *expect_ws(TokenType type, diags::Diagnostic msg);
    Token *advance_ws();

    /** 
     * Skip all tokens until a new declaration 
     * (new line or semicolon tells this)
     */
    void next_decl();
    /** Skip all new lines and semicolons until a new token */
    void skip_ends();
    /** Skip all new lines until some other token */
    void skip_nls();
    /** Skip `max` new lines */
    void skip_nls(unsigned max);

    ustring get_last_id(ir::Expression *e);
    bool is_id_or_scope(ir::Expression *e);

    /** 
     * Unescapes a string (for printing and so on) using moss string
     * escape sequences
     * 
     * @throw ir::Raise if incorrect escape sequence is used
     */
    ustring unescapeString(ustring str);

    void scan_line();

    template<typename ... Args>
    inline diags::Diagnostic create_diag(diags::DiagID id, Args ... args) {
        return diags::Diagnostic(this->src_file, tokens[curr_token], scanner, id, args ...);
    }

    void parser_error(diags::Diagnostic err_msg);
public:
    Parser(SourceFile &file) : src_file(file), scanner(new Scanner(file)), curr_token(0),
                               lower_range_prec(false), reading_by_lines(false),
                               multi_line_parsing(0) {}
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