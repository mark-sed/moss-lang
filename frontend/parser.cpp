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
        case TokenType::DOT: return Operator(OperatorKind::OP_ACCESS);
        case TokenType::SCOPE: return Operator(OperatorKind::OP_SCOPE);
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
                error::exit(error::ErrorCode::RUNTIME);
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
    if (m->empty() || !isa<EndOfFile>(m->back()))
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
 * ```
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
 * Expression -> ...
 * 
 * Annotation -> @|@! Expression
 * 
 * Block -> { Declaration* }
 * Body -> Block | Expression
 * 
 * If -> if ( Expression ) Body Elif* Else?
 * Elif -> elif ( Expression ) Body
 * Else -> else Body
 * 
 * While -> while ( Expression ) Body
 * 
 * DoWhile -> do Body while ( Expression )
 * 
 * For -> for ( Expression : Expression ) Body
 * 
 * Try -> try Body Catch+ Finally?
 * Catch -> catch ( Expression ) Body
 *        | catch ( Expression : Expression ) Body
 * Finally -> finally Body
 * 
 * Space -> space ID? Block
 * 
 * Import -> import ImportList
 * ImportList -> ImportName
 *             | ImportName as ID
 *             | ImportList , ImportName
 *             | ImportList , ImportName as ID
 * ImportName -> ID
 *             | SCOPE
 *             | ImportName :: *
 * 
 * Switch ->
 * Function ->
 * Constructor ->
 * Enum ->
 * Class -> 
 * ```
 */
IR *Parser::declaration() {
    LOGMAX("Parsing declaration");
    IR *decl = nullptr;
    // In case we errored out inside of function call, reset range
    // precedence lowering
    lower_range_prec = false;
    bool no_end_needed = false;

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
    // TODO: Tie annotation to proper IR
    if (match(TokenType::OUT_ANNOTATION)) {
        auto expr = expression();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        decl = new Annotation(expr, false);
    }
    else if (match(TokenType::IN_ANNOTATION)) {
        auto expr = expression();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        decl = new Annotation(expr, true);
    }

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
    else if (match(TokenType::ASSERT)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        std::vector<Expression *> args = arg_list();
        parser_assert((args.size() == 1 || args.size() == 2), create_diag(diags::ASSERT_EXPECTS_ARG));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        decl = new Assert(args[0], args.size() == 2 ? args[1] : nullptr);
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
    else if (match(TokenType::SPACE)) {
        if (check(TokenType::ID)) {
            auto name = advance();
            auto body = block();
            decl = new Space(name->get_value(), body);
            no_end_needed = true;
        }
        else if (check(TokenType::LEFT_CURLY)) {
            auto body = block();
            decl = new Space("", body);
            no_end_needed = true;
        }
        else {
            parser_error(create_diag(diags::MISSING_SPACE_BODY));
        }
    }

    // fun

    // expression
    else if (auto expr = expression()) {
        decl = expr;
    }

    // Every declaration has to end with nl or semicolon or eof
    if(!no_end_needed && !match(TokenType::END_NL) && !match(TokenType::END) && !check(TokenType::END_OF_FILE)) {
        // Dangling ')'
        if (match(TokenType::RIGHT_PAREN)) {
            parser_error(create_diag(diags::UNMATCHED_RIGHT_PAREN));
        }
        else {
            parser_error(create_diag(diags::EXPECTED_END));
        }
    }
    LOGMAX("Parsed declaration " << *decl);
    return decl;
}

std::list<ir::IR *> Parser::block() {
    // Skip new lines
    while(match(TokenType::END_NL))
        ; // Skipping empty new lines
    expect(TokenType::LEFT_CURLY, create_diag(diags::MISSING_BLOCK));
    while(match(TokenType::END_NL))
        ; // Skipping empty new lines
    std::list<ir::IR *> decls;

    IR *decl = nullptr;
    while (!check(TokenType::RIGHT_CURLY) && !check(TokenType::END_OF_FILE)) {
        decl = declaration();
        if (!decl)
            break;
        decls.push_back(decl);
        skip_ends(); // Skip here so that we can check for } or eof
    }

    if (!match(TokenType::RIGHT_CURLY)) {
        parser_error(create_diag(diags::MISSING_RIGHT_CURLY));
    }

    return decls;
}

/** 
 * Note: Expression prioritizes argument list comma over range comma, meaning
 *       that when calling foo(1,2..4), this will result in foo(1,(2..4)).
 * 
 * ```
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
 * Range -> Expression , Expression .. Expression
 *        | Expression .. Expression
 * 
 * Call -> Expression ( ArgList )
 * 
 * ArgList -> 
 *          | Expression
 *          | ArgList , Expression
 * ```
 */
Expression *Parser::expression() {
    Expression *expr = silent();
    return expr;
}

Expression *Parser::silent() {
    bool is_silent = false;
    while (match(TokenType::SILENT)) {
        if (is_silent)
            parser_error(create_diag(diags::CANNOT_CHAIN_SILENT));
        is_silent = true;
        // Calling unpack makes this left associative, but this operator
        // Cannot be chained and has the lowest precedence
        auto expr = unpack();
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
        auto right = assignment();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right,  Operator(OperatorKind::OP_SET));
    }
    // TODO: Check for any compound assignment chains, which doesn't make sense
    // Current check only check if this is not after =
    while (check({TokenType::SET_CONCAT, TokenType::SET_EXP, TokenType::SET_PLUS,
                  TokenType::SET_MINUS, TokenType::SET_DIV, TokenType::SET_MUL, TokenType::SET_MOD})) {
        auto op = advance();
        if (is_set) {
            parser_error(create_diag(diags::CHAINED_COMPOUND_ASSIGN, op->get_cstr()));
        }
        is_set = true;
        auto right = assignment();
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
        auto val_true = ternary_if(); // Right associative
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
        auto expr = op_not(); // Right associative
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_NOT));
    }

    return eq_neq();
}

Expression *Parser::eq_neq() {
    Expression *expr = compare_gl();

    while (check({TokenType::EQ, TokenType::NEQ})) {
        auto op = advance();
        // Note: 'not' cannot be RHS without parenthesis
        // TODO: Add error note for this
        auto right = compare_gl();
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

    if (match(TokenType::IN)) {
        auto right = range(); // Chaining in is not allowed, parenthesis have to be used
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        return new BinaryExpr(expr, right, Operator(OperatorKind::OP_IN));
    }

    return expr;
}

Expression *Parser::range() {
    Expression *expr = concatenation();

    // expr..end
    // TODO: Raise a warning if in fun args and not in parenthesis
    if (match(TokenType::RANGE)) {
        auto end = concatenation();
        parser_assert(end, create_diag(diags::EXPR_EXPECTED));
        return new Range(expr, end);
    }
    // expr,second..end
    else if (!lower_range_prec && match(TokenType::COMMA)) {
        auto second = concatenation();
        parser_assert(second, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RANGE, create_diag(diags::RANGE_EXPECTED));
        auto end = concatenation();
        parser_assert(second, create_diag(diags::EXPR_EXPECTED));
        return new Range(expr, end, second);
    }

    return expr;
}

Expression *Parser::concatenation() {
    Expression *expr = add_sub();

    while (match(TokenType::CONCAT)) {
        auto right = add_sub();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, Operator(OperatorKind::OP_CONCAT));
    }

    return expr;
}

Expression *Parser::add_sub() {
    Expression *expr = mul_div_mod();

    while (check({TokenType::PLUS, TokenType::MINUS})) {
        auto op = advance();
        auto right = mul_div_mod();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::mul_div_mod() {
    Expression *expr = exponentiation();

    while (check({TokenType::MUL, TokenType::DIV, TokenType::MOD})) {
        auto op = advance();
        auto right = exponentiation();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::exponentiation() {
    Expression *expr = unary_plus_minus();

    while (match(TokenType::EXP)) {
        auto right = exponentiation(); // Right associative, so call itself
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, Operator(OperatorKind::OP_EXP));
    }

    return expr;
}

Expression *Parser::unary_plus_minus() {
    if (match(TokenType::MINUS)) {
        auto expr = unary_plus_minus(); // Right associative
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_MINUS));
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

    while (match(TokenType::DOT)) {
        auto elem = subscript();
        parser_assert(elem, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, elem, Operator(OperatorKind::OP_ACCESS));
    }

    return expr;
}

// Subscript or slice
Expression *Parser::subscript() {
    Expression *expr = call();

    while (match(TokenType::LEFT_SQUARE)) {
        auto index = ternary_if();
        parser_assert(index, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
        expr = new BinaryExpr(expr, index, Operator(OperatorKind::OP_SUBSC));
    }

    return expr;
}

std::vector<ir::Expression *> Parser::arg_list() {
    std::vector<ir::Expression *> args;
    // Setting this to true will indicate to not give precedence to range over
    // another argument
    lower_range_prec = true;

    Expression *expr = nullptr;
    do {
        expr = expression();
        if (expr)
            args.push_back(expr);
    } while (match(TokenType::COMMA) && expr);

    lower_range_prec = false;

    return args;
}

Expression *Parser::call() {
    Expression *expr = note();

    while (match(TokenType::LEFT_PAREN)) {
        auto args = arg_list();
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        expr = new Call(expr, args);
    } 

    return expr;
}

ir::Expression *Parser::note() {
    Expression *expr = scope();

    if (check(TokenType::STRING)) {
        auto val = advance();
        return new Note(expr, new StringLiteral(unescapeString(val->get_value())));
    }

    return expr;
}

Expression *Parser::scope() {
    Expression *expr = constant();

    while (match(TokenType::SCOPE)) {
        auto elem = constant();
        parser_assert(elem, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, elem, Operator(OperatorKind::OP_SCOPE));
    }

    return expr;
}

Expression *Parser::constant() {
    if (match(TokenType::LEFT_PAREN)) {
        bool prev_fun_args_state = lower_range_prec;
        lower_range_prec = false; // This will allow for range in function call
        auto expr = ternary_if();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        lower_range_prec = prev_fun_args_state;
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
        return new StringLiteral(unescapeString(val->get_value()));
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
    return nullptr;
}

ustring Parser::unescapeString(ustring str) {
    std::stringstream res;
    bool backslash = false;
    for(int i = 0; str[i] != '\0'; ++i) {
        char c = str[i];
        if(backslash) {
            switch(c) {
            case '\"': 
                res << '\"';
            break;
            case '\\': 
                res << '\\';
            break;
            case 'a': 
                res << '\a';
            break;
            case 'b': 
                res << '\b';
            break;
            case 'f': 
                res << '\f';
            break;
            case 'n': 
                res << '\n';
            break;
            case 'r': 
                res << '\r';
            break;
            case 't': 
                res << '\t';
            break;
            case 'v': 
                res << '\v';
            break;
            // TODO: https://en.cppreference.com/w/cpp/language/escape
            case 'x':
            case 'X':
                assert(false && "Hexadecimal escape sequences are not yet implemented");
            break;
            case 'q':
            case 'Q':
                assert(false && "Octal escape sequences are not yet implemented");
            break;
            case 'u':
            case 'U':
                assert(false && "Unicode escape sequences are not yet implemented");
            break;
            case 'N': // TODO: Should this be kept?
                assert(false && "Named unicode escape sequences are not yet implemented");
            break;
            default:
                parser_error(create_diag(diags::UNKNOWN_ESC_SEQ, c));
            }
            backslash = false;
            continue;
        }
        if(c == '\\') {
            backslash = true;
            continue;
        }
        res << c;
    }
    return res.str();
}

#undef parser_assert