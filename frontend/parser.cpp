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

static Operator token2operator(TokenType t) {
    switch(t) {
        case TokenType::CONCAT: return Operator(OperatorKind::OP_CONCAT);
        case TokenType::EXP: return Operator(OperatorKind::OP_EXP);
        case TokenType::PLUS: return Operator(OperatorKind::OP_PLUS);
        case TokenType::MINUS: return Operator(OperatorKind::OP_MINUS);
        case TokenType::DIV: return Operator(OperatorKind::OP_DIV);
        case TokenType::MUL: return Operator(OperatorKind::OP_MUL);
        case TokenType::MOD: return Operator(OperatorKind::OP_MOD);
        case TokenType::SET: return Operator(OperatorKind::OP_SET);
        // For compound assignment return the operator without assignment 
        case TokenType::SET_CONCAT: return Operator(OperatorKind::OP_SET_CONCAT);
        case TokenType::SET_EXP: return Operator(OperatorKind::OP_SET_EXP);
        case TokenType::SET_PLUS: return Operator(OperatorKind::OP_SET_PLUS);
        case TokenType::SET_MINUS: return Operator(OperatorKind::OP_SET_MINUS);
        case TokenType::SET_DIV: return Operator(OperatorKind::OP_SET_DIV);
        case TokenType::SET_MUL: return Operator(OperatorKind::OP_SET_MUL);
        case TokenType::SET_MOD: return Operator(OperatorKind::OP_SET_MOD);
        case TokenType::EQ: return Operator(OperatorKind::OP_EQ);
        case TokenType::NEQ: return Operator(OperatorKind::OP_NEQ);
        case TokenType::BT: return Operator(OperatorKind::OP_BT);
        case TokenType::LT: return Operator(OperatorKind::OP_LT);
        case TokenType::BEQ: return Operator(OperatorKind::OP_BEQ);
        case TokenType::LEQ: return Operator(OperatorKind::OP_LEQ);
        case TokenType::SHORT_C_AND: return Operator(OperatorKind::OP_SHORT_C_AND);
        case TokenType::SHORT_C_OR: return Operator(OperatorKind::OP_SHORT_C_OR);
        case TokenType::AND: return Operator(OperatorKind::OP_AND);
        case TokenType::OR: return Operator(OperatorKind::OP_OR);
        case TokenType::NOT: return Operator(OperatorKind::OP_NOT);
        case TokenType::XOR: return Operator(OperatorKind::OP_XOR);
        case TokenType::IN: return Operator(OperatorKind::OP_IN);
        /*case TokenType::DOT: return Operator(OperatorKind::OP_DOT);
        case TokenType::RANGE: return Operator(OperatorKind::OP_RANGE);
        case TokenType::SCOPE: return Operator(OperatorKind::OP_SCOPE);*/
        default:
            assert(false && "Token is not an operator");
            return Operator(OperatorKind::OP_UNKNOWN);
    }
}

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
        //LOGMAX(*t);
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

bool Parser::check(std::initializer_list<TokenType> types) {
    for (auto t : types) {
        if (check(t)) return true;
    }
    return false;
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

/**
 * EndOfFile -> EOF
 * 
 * Raise     -> ERROR_TOKEN
 *            | RAISE Expression
 * 
 * Assert    -> ASSERT '(' Expression ')'
 *            | ASSERT '(' Expression ',' Expression ')'
 * 
 * Return    -> RETURN Expression?
 * Break     -> BREAK
 * Continue  -> CONTINUE
 * 
 * Silent    -> '~' Expression
 * 
 * Expression -> ...
 */
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
    else if (auto expr = expression()) {
        decl = expr;
    }
    /*else if (match(TokenType::SILENT)) {
        auto expr = expression();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED_NOTE, "only expressions are outputted and can be silenced"));
        decl = new Silent(expr);
    }*/

    // Every declaration has to end with nl or semicolon or eof
    if(!match(TokenType::END_NL) && !match(TokenType::END) && !check(TokenType::END_OF_FILE)) {
        parser_error(create_diag(diags::EXPECTED_END));
    }
    LOGMAX("Parsed declaration " << *decl);
    return decl;
}

/** 
 * 
 * Expression -> ( Expression )
 * 
 * UnaryExpr -> MINUS Expression
 *            | NOT Expression
 *            | << Expression
 *            | ~ Expression
 * 
 * Constant -> ID
 *           | true | false | nil
 *           | NUMBER
 *           | FLOAT
 *           | STRING
 * 
 * BinaryExpr -> Expression (::|+|-|*|^|/|%|++) Expression
 *             | Expression (&&|'||'|and|or|xor|in) Expression
 *             | Expression (==|!=|>|<|<=|>=) Expression
 *             | Expression = Expression
 * 
 * TernaryIf -> Expression ? Expression : Expression
 * 
 */
Expression *Parser::expression() {
    Expression *expr = silent();
    return expr;
}

Expression *Parser::silent() {
    if (match(TokenType::SILENT)) {
        auto expr = ternary_if(); // Assignment does not return a value for output
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_SILENT));
    }

    return assignment();
}

Expression *Parser::assignment() {
    Expression *expr = unpack();

    bool is_set = false;
    while (match(TokenType::SET)) {
        is_set = true;
        auto right = ternary_if();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right,  Operator(OperatorKind::OP_SET));
    }
    // We use while to check for any compound assignment chains, which doesn't make sense
    while (check({TokenType::SET_CONCAT, TokenType::SET_EXP, TokenType::SET_PLUS,
                  TokenType::SET_MINUS, TokenType::SET_DIV, TokenType::SET_MUL, TokenType::SET_MOD})) {
        auto op = advance();
        if (is_set) {
            parser_error(create_diag(diags::CHAINED_COMPOUND_ASSIGN, op->get_cstr()));
        }
        is_set = true;
        auto right = ternary_if();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        // We cannot transform this into set and operation as then the left and
        // right pointers would be the same
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::unpack() {
    if (match(TokenType::UNPACK)) {
        auto expr = ternary_if();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_UNPACK));
    }

    return ternary_if();
}

Expression *Parser::ternary_if() {
    Expression *expr = short_circuit();

    if (match(TokenType::QUESTION_M)) {
        auto val_true = ternary_if();
        parser_assert(val_true, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::COLON, create_diag(diags::TERNARY_IF_MISSING_FALSE));
        auto val_false = ternary_if();
        parser_assert(val_false, create_diag(diags::EXPR_EXPECTED));
        return new TernaryIf(expr, val_true, val_false);
    }

    return expr;
}

Expression *Parser::short_circuit() {
    Expression *expr = and_or_xor();

    while (check({TokenType::SHORT_C_AND, TokenType::SHORT_C_OR})) {
        auto op = advance();
        auto right = and_or_xor();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::and_or_xor() {
    Expression *expr = op_not();

    while (check({TokenType::AND, TokenType::OR, TokenType::XOR})) {
        auto op = advance();
        auto right = op_not();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::op_not() {
    if (match(TokenType::NOT)) {
        auto expr = op_not();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_NOT));
    }

    return eq_neq();
}

Expression *Parser::eq_neq() {
    Expression *expr = compare_gl();

    while (check({TokenType::EQ, TokenType::NEQ})) {
        auto op = advance();
        auto right = op_not();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::compare_gl() {
    Expression *expr = membership();

    while (check({TokenType::LEQ, TokenType::BEQ, TokenType::LT, TokenType::BT})) {
        auto op = advance();
        auto right = membership();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::membership() {
    Expression *expr = range();

    return expr;
}

Expression *Parser::range() {
    Expression *expr = concatenation();

    return expr;
}

Expression *Parser::concatenation() {
    Expression *expr = add_sub();

    return expr;
}

Expression *Parser::add_sub() {
    Expression *expr = mul_div_mod();

    return expr;
}

Expression *Parser::mul_div_mod() {
    Expression *expr = exponentiation();

    return expr;
}

Expression *Parser::exponentiation() {
    Expression *expr = unary_plus_minus();

    return expr;
}

Expression *Parser::unary_plus_minus() {
    if (match(TokenType::MINUS)) {
        LOGMAX("Parsed unary minus");
        auto expr = unary_plus_minus();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_NEG));
    }
    else if (match(TokenType::PLUS)) {
        auto expr = unary_plus_minus();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return expr;
    }

    return element_access();
}

Expression *Parser::element_access() {
    Expression *expr = subscript();

    return expr;
}

Expression *Parser::subscript() {
    Expression *expr = call();

    return expr;
}

Expression *Parser::call() {
    Expression *expr = scope();

    return expr;
}

Expression *Parser::scope() {
    Expression *expr = constant();

    return expr;
}


Expression *Parser::constant() {
    LOGMAX("Parsing Constant");
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = expression();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        return expr;
    }
    else if (check(TokenType::ID)) {
        auto id = advance();
        return new Variable(id->get_value());
    }
    else if (check(TokenType::INT)) {
        auto val = advance();
        // The value was parsed and checked, so no need to check for
        // correct conversion
        return new IntLiteral(atol(val->get_value().c_str()));
    }
    else if (check(TokenType::FLOAT)) {
        auto val = advance();
        // The value was parsed and checked, so no need to check for
        // correct conversion
        return new FloatLiteral(std::stod(val->get_value()));
    }
    else if (check(TokenType::STRING)) {
        auto val = advance();
        return new StringLiteral(val->get_value());
    }
    else if (match(TokenType::TRUE)) {
        return new BoolLiteral(true);
    }
    else if (match(TokenType::FALSE)) {
        return new BoolLiteral(false);
    }
    else if (match(TokenType::NIL)) {
        return new NilLiteral();
    }
}

#undef parser_assert