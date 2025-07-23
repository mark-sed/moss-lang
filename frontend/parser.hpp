/// 
/// \file parser.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Semantical analysis of code and AST generation
/// 

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

/// This macro asserts that condition is true otherwise it raises a parser_error
#define parser_assert(cond, msg) do { if(!(cond)) parser_error(msg); } while(0)

void parser_error(diags::Diagnostic err_msg);

/// \brief Moss token parsing into AST
/// Parser can parse the whole file at once or go line by line. 
class Parser {
private:
    SourceFile &src_file;
    Scanner *scanner;
    
    size_t curr_token;
    std::vector<Token *> tokens;

    int lower_range_prec;
    bool reading_by_lines;
    int multi_line_parsing;
    bool enable_code_output;

    std::list<ir::IR *> parents;
    std::list<ir::Annotation *> outter_annots;
 
    /// Parses an IR that is a declaration (if not throws an error)
    /// Declaration is a statement that can be on its own, so like a line in REPL
    /// 
    /// \note This will ignore all standalone new lines and semicolons
    /// \throw If error is encountered, this throws ir::Raise describing this
    ///        error, which can be interpreted by moss
    /// \return Parsed input into an IR class 
    ir::IR *declaration();
    
    bool bind_docstring();
    void check_code_output(ir::Module *m, ir::IR *decl);

    /// Tries to parse an expression
    /// 
    /// \throw If error is encountered, this throws ir::Raise describing this
    ///        error, which can be interpreted by moss
    /// \return Parsed input into an Expression class or nullptr if there is
    ///         not an expression on the input
    ir::Expression *expression(bool allow_set=false);

    /// Tries to parse an annotation
    ir::Annotation *annotation();

    /// Tries to parse a block of code
    std::list<ir::IR *> block(); 
    /// Tries to parse a body,
    /// which might be a block of code or a declaration
    std::list<ir::IR *> body();

    ir::Expression *constant();
    std::vector<ir::Expression *> expr_list(bool only_scope_or_id=false, bool allow_set=false);
    std::vector<ir::Argument *> arg_list();
    ir::OperatorLiteral *operator_name();
    ir::Argument *argument(bool allow_default_value=false);
    std::list<ir::IR *> cases();
    ir::Expression *list_of_vars(ir::Expression *first, ir::Expression *second, int rest_index);
    ir::Expression *fstring(FStringToken *fstr);

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
    ir::Expression *call_access_subs(bool allow_star=false);
    ir::Expression *note();
    ir::Expression *scope();

    bool check(TokenType type);
    bool check(std::initializer_list<TokenType> types);
    bool match(TokenType type);
    Token *expect(TokenType type, diags::Diagnostic msg);
    Token *advance();
    Token *peek(int offset=1);
    Token *peek_ws(int offset=1);
    void put_back();

    bool check_ws(TokenType type);
    bool match_ws(TokenType type);
    Token *expect_ws(TokenType type, diags::Diagnostic msg);
    Token *advance_ws();
 
    /// Skip all tokens until a new declaration 
    /// (new line or semicolon tells this)
    void next_decl();
    /// Skip all new lines and semicolons until a new token
    void skip_ends();
    /// Skips all new lines, semicolons and white spaces until a new token
    void skip_ends_and_ws();
    /// Skip all new lines until some other token
    void skip_nls();
    /// Skip `max` new lines
    void skip_nls(unsigned max);

    ustring get_last_id(ir::Expression *e);
    bool is_id_or_member(ir::Expression *e);
 
    /// Unescapes a string (for printing and so on) using moss string
    /// escape sequences
    /// 
    /// \throw ir::Raise if incorrect escape sequence is used 
    ustring unescapeString(ustring str);

    /// Adds new tokens to the current token list
    void spill_tokens(std::list<Token *> ts) {
        this->tokens.insert(tokens.begin()+curr_token, ts.begin(), ts.end());
    }

    void scan_line();

    /// \return SourceInfo for the currently parsed token
    SourceInfo curr_src_info() {
        assert(curr_token < tokens.size() && "oob");
        return tokens[curr_token]->get_src_info();
    }

    template<typename ... Args>
    inline diags::Diagnostic create_diag(diags::DiagID id, Args ... args) {
        return diags::Diagnostic(this->src_file, tokens[curr_token]->get_src_info(), scanner, id, args ...);
    }

    template<typename ... Args>
    inline diags::Diagnostic create_diag(diags::DiagID id, SourceInfo src_info, Args ... args) {
        return diags::Diagnostic(this->src_file, src_info, scanner, id, args ...);
    }
public:
    template<typename ... Args>
    inline diags::Diagnostic create_diag(SourceInfo src_info, diags::DiagID id, Args ... args) {
        return diags::Diagnostic(this->src_file, src_info, scanner, id, args ...);
    }

    Parser(SourceFile &file) : src_file(file), scanner(new Scanner(file)), curr_token(0),
                               lower_range_prec(false), reading_by_lines(false),
                               multi_line_parsing(0), enable_code_output(false) {}
    ~Parser() {
        delete scanner;
        for (auto *t: tokens)
            delete t;
    }

    /// Parses a moss file into an AST
    /// \return File parsed into a ir::Module. If there is an error then ir::Raise is thrown
    /// \throw ir::Raise with parser/scanner error to be rethrown by moss
    ir::IR *parse();

    /// Parses a single line or multiple ones if it is a multi-line declaration into an AST
    /// \return Vector of parsed IRs. If there is an error then ir::Raise is thrown
    /// \throw ir::Raise with parser/scanner error to be rethrown by moss
    std::vector<ir::IR *> parse_line();

    SourceFile &get_src_file() { return this->src_file; }
    Scanner *get_scanner() { return this->scanner; }
};

}

#endif//_PARSER_HPP_