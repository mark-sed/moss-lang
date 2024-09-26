#include "parser.hpp"
#include "ir.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "errors.hpp"
#include "clopts.hpp"
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
    reading_by_lines = false;
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

void Parser::scan_line() {
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
}

std::vector<ir::IR *> Parser::parse_line() {
    std::vector<ir::IR *> line_decls;
    reading_by_lines = true;
    tokens.clear();
    curr_token = 0;
    // Scan one line (this fills up tokens)
    scan_line();

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
#ifndef NDEBUG
            // In case of parse-only mode in repl we want to print the error
            // messages so that errors are visible even without logging
            if (clopts::parse_only) {
                StringLiteral *err_msg = dyn_cast<StringLiteral>(raise->get_exception());
                errs << err_msg->get_value();
            }
#endif
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
    if (multi_line_parsing && reading_by_lines && tokens[curr_token]->get_type() == TokenType::END_NL) {
        // We have reached new line in parsing by lines, but we are inside of
        // a multiline structure - scan another line
        scan_line();
    }
    else if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE ||
        (reading_by_lines && tokens[curr_token]->get_type() == TokenType::END_NL)) {
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
    if (multi_line_parsing && reading_by_lines && tokens[curr_token]->get_type() == TokenType::END_NL) {
        // We have reached new line in parsing by lines, but we are inside of
        // a multiline structure - scan another line
        scan_line();
    }
    else if (tokens[curr_token]->get_type() == TokenType::END_OF_FILE ||
        (reading_by_lines && tokens[curr_token]->get_type() == TokenType::END_NL)) {
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
    if (!check(TokenType::END_NL) && !check(TokenType::END))
        return;
    ++multi_line_parsing;
    while(match(TokenType::END) || match(TokenType::END_NL))
        ; // Skipping empty new lines and ;
    --multi_line_parsing;
}

void Parser::skip_nls() {
    if (!check(TokenType::END_NL))
        return;
    ++multi_line_parsing;
    while(match(TokenType::END_NL))
        ; // Skipping empty new lines
    --multi_line_parsing;
}

void Parser::skip_nls(unsigned max) {
    ++multi_line_parsing;
    while(max > 0 && match(TokenType::END_NL))
        --max; // Skipping empty new lines
    --multi_line_parsing;
}

ustring Parser::get_last_id(Expression *e) {
    assert(e && "nullptr passed for check");
    if (auto v = dyn_cast<Variable>(e)) {
        return v->get_name();
    }
    if (auto be = dyn_cast<BinaryExpr>(e)) {
        if (!isa<BinaryExpr>(be->get_left()) && !isa<Variable>(be->get_left()))
            parser_error(create_diag(diags::SCOPE_OR_ID_EXPECTED));
        if (be->get_op().get_kind() == OperatorKind::OP_SCOPE) {
            return get_last_id(be->get_right());
        }
        else {
            parser_error(create_diag(diags::SCOPE_OR_ID_EXPECTED));
        }
    }
    parser_error(create_diag(diags::SCOPE_OR_ID_EXPECTED));
    return "";
}

bool Parser::is_id_or_scope(Expression *e) {
    assert(e && "nullptr passed for check");
    if (isa<Variable>(e)) {
        return true;
    }
    if (auto be = dyn_cast<BinaryExpr>(e)) {
        if (!isa<BinaryExpr>(be->get_left()) && !isa<Variable>(be->get_left()))
            return false;
        if (be->get_op().get_kind() == OperatorKind::OP_SCOPE) {
            return is_id_or_scope(be->get_right());
        }
        return false;
    }
    return false;
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
 * Annotation -> (@|@!) Expression
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
 * 
 * IdList -> ID
 *         | IdList (,|;|\n) ID
 * Enum -> enum { IdList }
 * 
 * Class -> 
 * ```
 */
IR *Parser::declaration() {
    LOGMAX("Parsing declaration");
    IR *decl = nullptr;
    // In case we errored out inside of function call, reset range
    // precedence lowering
    lower_range_prec = false;
    // Multiline parsing cannot be zeroes in there as this might be called
    // in multiline declaration
    assert(multi_line_parsing >= 0 && "Got out of multiline structures more than into them?");
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
    else if (match(TokenType::IMPORT)) {
        std::vector<ir::Expression *> names;
        std::vector<ustring> aliases;
        lower_range_prec = true;

        Expression *name = nullptr;
        do {
            if (name)
                skip_nls();
            name = expression();
            parser_assert(name, create_diag(diags::SCOPE_OR_ID_EXPECTED));
            names.push_back(name);
            // We always call get_last_id as it checks that the name is ID or Scope
            ustring alias = get_last_id(name);
            if (match(TokenType::AS)) {
                auto alias_tok = expect(TokenType::ID, create_diag(diags::ID_EXPECTED));
                alias = alias_tok->get_value();
            }
            aliases.push_back(alias);
        } while (match(TokenType::COMMA) && name);

        lower_range_prec = false;
        decl = new Import(names, aliases);
    }

    // if
    else if (match(TokenType::IF)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::IF_REQUIRES_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        auto ifbody = body(); // Body asserts for missing declaration
        // In script you can have new lines in REPL only 1
        if (!reading_by_lines)
            skip_nls();
        else {
            skip_nls(1);
        }
        if (match(TokenType::ELSE)) {
            auto elsebody = body();
            decl = new If(cond, ifbody, new Else(elsebody));
        }
        else {
            decl = new If(cond, ifbody);
        }
        // This is true even for single declaration as that declaration itself
        // has to have an end. No need for second one.
        no_end_needed = true;
    }
    else if (match(TokenType::ELSE)) {
        parser_error(create_diag(diags::ELSE_WITHOUT_IF));
    }

    // while / do while
    else if (match(TokenType::WHILE)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::WHILE_REQUIRES_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        auto whbody = body();
        decl = new While(cond, whbody);
        no_end_needed = true;
    }
    else if (match(TokenType::DO)) {
        auto whbody = body();
        skip_nls();
        expect(TokenType::WHILE, create_diag(diags::NO_WHILE_AFTER_DO));
        expect(TokenType::LEFT_PAREN, create_diag(diags::WHILE_REQUIRES_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        decl = new DoWhile(cond, whbody);
    }

    // for
    else if (match(TokenType::FOR)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::FOR_REQUIRES_PARENTH));
        auto iterator = expression();
        parser_assert(iterator, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::COLON, create_diag(diags::FOR_MISSING_COLON));
        auto collection = expression();
        parser_assert(collection, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        auto frbody = body();
        decl = new ForLoop(iterator, collection, frbody);
        no_end_needed = true;
    }

    // switch
    else if (match(TokenType::SWITCH)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::SWITCH_REQUIRES_PARENTH));
        auto val = expression();
        parser_assert(val, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        ++multi_line_parsing;
        auto sw_cases = cases();
        no_end_needed = true;
        --multi_line_parsing;
        decl = new Switch(val, sw_cases);
    }
    else if (match(TokenType::CASE)) {
        parser_error(create_diag(diags::CASE_OUTSIDE_SWITCH));
    }
    else if (match(TokenType::DEFAULT)) {
        parser_error(create_diag(diags::DEFAULT_OUTSIDE_SWITCH));
    }

    // try
    // TODO: Should we allow try ... finally without catch as Python does?
    else if (match(TokenType::TRY)) {
        auto trbody = body();
        skip_nls();
        std::vector<Catch *> catches;
        while (match(TokenType::CATCH)) {
            expect(TokenType::LEFT_PAREN, create_diag(diags::CATCH_REQUIRES_PARENTH));
            auto arg = argument();
            parser_assert(arg, create_diag(diags::EXPR_EXPECTED));
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
            auto ctbody = body();
            catches.push_back(new Catch(arg, ctbody));
            if (!reading_by_lines)
                skip_nls();
            else {
                skip_nls(1);
            }
        }
        if (catches.empty())
            parser_error(create_diag(diags::CATCH_EXPECTED));
        if (!reading_by_lines)
            skip_nls();
        else {
            skip_nls(1);
        }
        Finally *finallyStmt = nullptr;
        if (match(TokenType::FINALLY)) {
            auto fnbody = body();
            finallyStmt = new Finally(fnbody);
        }
        decl = new Try(catches, trbody, finallyStmt);
        no_end_needed = true;
    }
    else if (match(TokenType::CATCH)) {
        parser_error(create_diag(diags::CATCH_WITHOUT_TRY));
    }
    else if (match(TokenType::FINALLY)) {
        parser_error(create_diag(diags::FINALLY_WITHOUT_TRY));
    }
    
    // assert / raise / return
    else if (match(TokenType::ASSERT)) {
        expect(TokenType::LEFT_PAREN, create_diag(diags::ASSERT_MISSING_PARENTH));
        std::vector<Expression *> args = expr_list();
        parser_assert((args.size() == 1 || args.size() == 2), create_diag(diags::ASSERT_EXPECTS_ARG));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
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
    else if (match(TokenType::ENUM)) {
        ++multi_line_parsing;
        auto name = expect(TokenType::ID, create_diag(diags::ENUM_REQUIRES_NAME));
        skip_nls();
        expect(TokenType::LEFT_CURLY, create_diag(diags::MISSING_BLOCK));
        skip_nls();
        std::vector<ustring> values;

        while (!check(TokenType::RIGHT_CURLY) && !check(TokenType::END_OF_FILE)) {
            auto val = expect(TokenType::ID, create_diag(diags::INCORRECT_ENUM_VALUE));
            auto val_str = val->get_value();
            if (std::find(values.begin(), values.end(), val_str) != values.end()) {
                parser_error(create_diag(diags::ENUM_VALUE_REDEFINITION, val_str.c_str(), name->get_value().c_str()));
            }
            values.push_back(val_str);
            if (!check({TokenType::COMMA, TokenType::END, TokenType::END_NL})) {
                if (check(TokenType::RIGHT_CURLY))
                    break;
                parser_error(create_diag(diags::MISSING_ENUM_SEPAR));
            }
            advance();
            skip_ends(); // Skip here so that we can check for } or eof
        }

        if (!match(TokenType::RIGHT_CURLY)) {
            parser_error(create_diag(diags::MISSING_RIGHT_CURLY));
        }

        --multi_line_parsing;
        decl = new Enum(name->get_value(), values);
        no_end_needed = true;
    }

    // class
    else if (match(TokenType::CLASS)) {
        ++multi_line_parsing;
        if (check(TokenType::ID)) {
            auto name = advance();
            std::vector<Expression *> parents;
            if (match(TokenType::COLON)) {
                parents = expr_list(true);
                parser_assert(!parents.empty(), create_diag(diags::PARENT_LIST_EXPECTED));
            }
            auto clbody = block();
            decl = new Class(name->get_value(), parents, clbody);
            no_end_needed = true;
        }
        else {
            parser_error(create_diag(diags::MISSING_CLASS_NAME));
        }
        --multi_line_parsing;
    }

    // space
    else if (match(TokenType::SPACE)) {
        ++multi_line_parsing;
        if (check(TokenType::ID)) {
            auto name = advance();
            auto stbody = block();
            decl = new Space(name->get_value(), stbody);
            no_end_needed = true;
        }
        else if (check(TokenType::LEFT_CURLY)) {
            auto stbody = block();
            decl = new Space("", stbody);
            no_end_needed = true;
        }
        else {
            parser_error(create_diag(diags::MISSING_SPACE_BODY));
        }
        --multi_line_parsing;
    }

    // fun / constructor / lambdas
    // TODO: Lambda - Probably a new IR that will be Expression?
    else if (check({TokenType::FUN, TokenType::NEW})) {
        auto funt = advance();
        bool constructor = funt->get_type() == TokenType::NEW;
        if (check(TokenType::ID)) {
            auto id = advance();
            expect(TokenType::LEFT_PAREN, create_diag(diags::FUN_REQUIRES_PARENTH));
            std::vector<Argument *> args;
            if (!match(TokenType::RIGHT_PAREN)) {
                args = arg_list();
                expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
            }
            skip_nls();
            if (check(TokenType::LEFT_CURLY)) {
                auto fnbody = block();
                decl = new Function(id->get_value(), args, fnbody, constructor);
            }
            else if (check(TokenType::SET)) {
                assert(false && "TODO: lambdas");
            }
            else {
                parser_error(create_diag(diags::MISSING_FUN_BODY));
            }
        }
        else if (match(TokenType::LEFT_PAREN)) {
            parser_assert(!constructor, create_diag(diags::MISSING_CONSTR_NAME));
            assert(false && "TODO: anon lambdas");
        }
        else {
            // TODO: If not constructor then fun args
            parser_error(create_diag(diags::ID_EXPECTED));
        }
    }

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
    assert(decl && "Nothing parsed and no raise?");
    return decl;
}

std::list<ir::IR *> Parser::block() {
    skip_nls();
    expect(TokenType::LEFT_CURLY, create_diag(diags::MISSING_BLOCK));
    skip_nls();
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

std::list<ir::IR *> Parser::body() {
    skip_nls();
    if (check(TokenType::LEFT_CURLY)) {
        ++multi_line_parsing;
        return block();
        --multi_line_parsing;
    }
    auto decl = declaration();
    parser_assert((decl), create_diag(diags::DECL_EXPECTED));
    if (isa<EndOfFile>(decl))
        parser_error(create_diag(diags::UNEXPECTED_EOF));
    std::list<ir::IR *> decls{decl};
    return decls;
}

std::list<ir::IR *> Parser::cases() {
    skip_nls();
    expect(TokenType::LEFT_CURLY, create_diag(diags::SWITCH_BODY_EXPECTED));
    skip_nls();
    std::list<ir::IR *> decls;

    bool found_default = false;
    while (!check(TokenType::RIGHT_CURLY) && !check(TokenType::END_OF_FILE)) {
        bool default_case = false;
        if (match(TokenType::DEFAULT)) {
            parser_assert(!found_default, create_diag(diags::MULTIPLE_DEFAULTS));
            found_default = default_case = true;
        }
        else {
            expect(TokenType::CASE, create_diag(diags::SWITCH_CASE_EXPECTED));
        }
        auto vals = expr_list();
        expect(TokenType::COLON, create_diag(diags::CASE_MISSING_COLON));
        auto swbody = body();
        decls.push_back(new Case(vals, swbody, default_case));
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
        parser_assert(expr, create_diag(diags::NO_LHS_FOR_SET));
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
        parser_assert(expr, create_diag(diags::NO_LHS_FOR_SET));
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
        parser_assert(expr, create_diag(diags::TERNARY_IF_MISSING_COND));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
        auto right = membership();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::membership() {
    Expression *expr = range();

    if (match(TokenType::IN)) {
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "in"));
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
        parser_assert(expr, create_diag(diags::NO_LHS_IN_RANGE));
        auto end = concatenation();
        parser_assert(end, create_diag(diags::EXPR_EXPECTED));
        return new Range(expr, end);
    }
    // expr,second..end
    else if (!lower_range_prec && match(TokenType::COMMA)) {
        parser_assert(expr, create_diag(diags::NO_LHS_IN_RANGE));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "++"));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
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
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, op->get_value().c_str()));
        auto right = exponentiation();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()));
    }

    return expr;
}

Expression *Parser::exponentiation() {
    Expression *expr = unary_plus_minus();

    while (match(TokenType::EXP)) {
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "^"));
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
        parser_assert(expr, create_diag(diags::NO_LHS_IN_ACCESS));
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
        parser_assert(expr, create_diag(diags::NO_LHS_IN_SUBSCRIPT));
        auto index = ternary_if();
        parser_assert(index, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
        expr = new BinaryExpr(expr, index, Operator(OperatorKind::OP_SUBSC));
    }

    return expr;
}

std::vector<ir::Expression *> Parser::expr_list(bool only_scope_or_id) {
    std::vector<ir::Expression *> args;
    // Setting this to true will indicate to not give precedence to range over
    // another argument
    lower_range_prec = true;

    Expression *expr = nullptr;
    do {
        skip_nls();
        expr = expression();
        if (expr) {
            if (only_scope_or_id)
                parser_assert(is_id_or_scope(expr), create_diag(diags::SCOPE_OR_ID_EXPECTED));
            args.push_back(expr);
        }
    } while (match(TokenType::COMMA) && expr);

    lower_range_prec = false;

    return args;
}

std::vector<ir::Argument *> Parser::arg_list() {
    std::vector<ir::Argument *> args;
    lower_range_prec = true;

    Argument *arg = nullptr;
    do {
        skip_nls();
        arg = argument(true);
        assert(arg && "Argument not returned");
        args.push_back(arg);
    } while (match(TokenType::COMMA));

    lower_range_prec = false;
    return args;
}

Expression *Parser::call() {
    Expression *expr = note();

    while (match(TokenType::LEFT_PAREN)) {
        // This assert should never be raised as ( would be matched in constant
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "()"));
        auto args = expr_list();
        skip_nls();
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        expr = new Call(expr, args);
    } 

    return expr;
}

ir::Expression *Parser::note() {
    Expression *expr = scope();

    if (check(TokenType::STRING)) {
        // expr should never be nullptr as string would be parsed in constant
        assert(expr && "Somehow string was trying to be parsed as a note without prefix");
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
        // Expr may be nullptr as that is the global scope
        if (!expr) {
            expr = new UnaryExpr(elem, Operator(OperatorKind::OP_SCOPE));
        }
        else {
            expr = new BinaryExpr(expr, elem, Operator(OperatorKind::OP_SCOPE));
        }
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

Argument *Parser::argument(bool allow_default_value) {
    if (check(TokenType::ID)) {
        auto id = advance();
        Expression *default_value = nullptr;
        std::vector<Expression *> types;
        if (match(TokenType::COLON)) {
            if (match(TokenType::LEFT_SQUARE)) {
                lower_range_prec = true;
                Expression *expr = nullptr;
                do {
                    skip_nls(); // TODO: Keep this?
                    expr = expression();
                    if (expr) {
                        parser_assert(is_id_or_scope(expr), create_diag(diags::SCOPE_OR_ID_EXPECTED));
                        types.push_back(expr);
                    }
                } while (match(TokenType::COMMA) && expr);
                parser_assert(!types.empty(), create_diag(diags::TYPE_EXPECTED));
                expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
                lower_range_prec = false;
            }
            else {
                auto type = scope();
                parser_assert(type, create_diag(diags::TYPE_EXPECTED));
                parser_assert(is_id_or_scope(type), create_diag(diags::SCOPE_OR_ID_EXPECTED));
                types.push_back(type);
            }
        }
        if (match(TokenType::SET)) {
            parser_assert(allow_default_value, create_diag(diags::DEFAULT_NOT_ALLOWED));
            default_value = expression();
            parser_assert(default_value, create_diag(diags::EXPR_EXPECTED));
        }
        return new Argument(id->get_value(), types, default_value);
    }
    else {
        parser_error(create_diag(diags::INCORRECT_ARGUMENT));
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