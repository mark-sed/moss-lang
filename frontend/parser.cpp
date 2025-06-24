#include "parser.hpp"
#include "ir.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "errors.hpp"
#include "clopts.hpp"
#include <cassert>
#include <climits>
#include <cmath>
#include <codecvt>
#include <locale>

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

void Parser::check_code_output(Module *m, ir::IR *decl) {
    if (enable_code_output && !isa<ir::Note>(decl)) {
        auto annt = dyn_cast<ir::Annotation>(decl);
        if (!annt || annt->get_name() != "enable_code_output") {
            std::stringstream code_str;
            code_str << "```\n";
            auto src_i = decl->get_src_info();
            for (unsigned l = src_i.get_lines().first; l <= src_i.get_lines().second; ++l) {
                code_str << scanner->get_src_text()[l] << "\n";
            }
            code_str << "```\n";
            m->push_back(new ir::Note("md", new ir::StringLiteral(code_str.str(), src_i), src_i));
        }
    }
}

IR *Parser::parse() {
    LOG1("Started parsing module");
    reading_by_lines = false;
    SourceInfo mod_src_i(src_file, 0, 0, 0, 0);
    Module *m = new Module(this->src_file.get_module_name(), mod_src_i);
    parents.push_back(m);

    LOG2("Running scanner");
    Token *t = nullptr;
    do {
        t = scanner->next_token();
        tokens.push_back(t);
    } while(t->get_type() != TokenType::END_OF_FILE);
    LOG2("Finished scanning");

    try {
        bind_docstring();
    } catch (Raise *raise) {
        delete m;
        return raise;
    }

    while (!check(TokenType::END_OF_FILE)) {
        IR *decl;
        try {
            decl = declaration();
        } catch (Raise *raise) {
            delete m;
            return raise;
        }
        assert(decl && "Declaration in parser is nullptr");
        check_code_output(m, decl);
        m->push_back(decl);
    }

    // Append EOF, but only when not already present as it may be added by
    // an empty last line
    if (m->empty() || !isa<EndOfFile>(m->back()))
        m->push_back(new ir::EndOfFile(curr_src_info()));
        
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

    while (!check({TokenType::END_NL,TokenType::END})) {
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

Token *Parser::peek(int offset) {
    if (curr_token + offset > tokens.size()) return tokens.back();
    int gen_off = 0;
    int i = 0;
    assert(offset >= 0);
    while (tokens[curr_token+i]->get_type() == TokenType::WS) {
        ++i;
    }
    while (gen_off != offset) {
        auto tt = tokens[curr_token+i]->get_type();
        ++i;
        if (tt == TokenType::WS) continue;
        ++gen_off;
    }
    return tokens[curr_token+i];
}

Token *Parser::peek_ws(int offset) {
    if (curr_token + offset > tokens.size()) return tokens.back();
    return tokens[curr_token+offset];
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

void Parser::put_back() {
    assert(curr_token > 0 && "take back before any advance");
    while (tokens[--curr_token]->get_type() == TokenType::WS) {
        assert(curr_token > 0 && "take back before any non-ws advance");
        --curr_token;
    }
}

void moss::parser_error(diags::Diagnostic err_msg) {
    // TODO: Change to specific exception child type (such as TypeError)
    auto str_msg = error::format_error(err_msg);
    SourceInfo msg_src;
    throw new Raise(new StringLiteral(str_msg, msg_src), msg_src);
}

void Parser::skip_ends() {
    if (!check(TokenType::END_NL) && !check(TokenType::END))
        return;
    ++multi_line_parsing;
    while(match(TokenType::END) || match(TokenType::END_NL))
        ; // Skipping empty new lines and ;
    --multi_line_parsing;
}

void Parser::skip_ends_and_ws() {
    if (!check(TokenType::END_NL) && !check(TokenType::END) && !check(TokenType::WS))
        return;
    ++multi_line_parsing;
    while(match(TokenType::END) || match(TokenType::END_NL) || match(TokenType::WS))
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
    if (isa<Variable>(e) || isa<AllSymbols>(e)) {
        return e->get_name();
    }
    if (auto be = dyn_cast<BinaryExpr>(e)) {
        if (!isa<BinaryExpr>(be->get_left()) && !isa<Variable>(be->get_left()) && !isa<UnaryExpr>(be->get_left())) 
            parser_error(create_diag(diags::MEMBER_OR_ID_EXPECTED));
        if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
            return get_last_id(be->get_right());
        }
        else {
            parser_error(create_diag(diags::MEMBER_OR_ID_EXPECTED));
        }
    }
    if (auto ue = dyn_cast<UnaryExpr>(e)) {
        // Only ::Name then raise an exception
        if (ue->get_op().get_kind() == OperatorKind::OP_SCOPE)
            parser_assert(!isa<Variable>(ue->get_expr()), create_diag(diags::SPACE_IMPORT_AS_ITSELF));
        return get_last_id(ue->get_expr());
    }
    parser_error(create_diag(diags::MEMBER_OR_ID_EXPECTED));
    return "";
}

bool Parser::is_id_or_member(Expression *e) {
    assert(e && "nullptr passed for check");
    if (isa<Variable>(e)) {
        return true;
    }
    if (auto be = dyn_cast<BinaryExpr>(e)) {
        if (!isa<BinaryExpr>(be->get_left()) && !isa<Variable>(be->get_left()))
            return false;
        if (be->get_op().get_kind() == OperatorKind::OP_ACCESS) {
            return is_id_or_member(be->get_right());
        }
        if (be->get_op().get_kind() == OperatorKind::OP_SUBSC) {
            return is_id_or_member(be->get_left());
        }
        return false;
    }
    if (auto ue = dyn_cast<UnaryExpr>(e)) {
        if (ue->get_op().get_kind() == OperatorKind::OP_SCOPE) {
            return is_id_or_member(ue->get_expr());
        }
    }
    return false;
}

ir::Annotation *Parser::annotation() {
    Annotation *decl = nullptr;
    // outer / inner annotation
    if (check({TokenType::OUT_ANNOTATION, TokenType::IN_ANNOTATION})) {
        bool inner = advance()->get_type() == TokenType::IN_ANNOTATION;
        auto name = expect(TokenType::ID, create_diag(diags::ANNOT_EXPECTS_ID_NAME));
        Expression *value = nullptr;
        if (match(TokenType::LEFT_PAREN)) {
            auto mul_vals = expr_list();
            parser_assert(!mul_vals.empty(), create_diag(diags::EXPR_EXPECTED));
            if (mul_vals.size() == 1)
                value = mul_vals[0];
            else
                value = new List(mul_vals, curr_src_info());
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        }
        
        // @!enable_code_output is also a parser annotation
        if (name->get_value() == "enable_code_output" && !value) {
            enable_code_output = true;
        } else if (name->get_value() == "disable_code_output" && !value) {
            enable_code_output = false;
        }

        if (!value)
            value = new NilLiteral(curr_src_info());
        decl = new Annotation(name->get_value(), value, inner, curr_src_info());
    }
    return decl;
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
 * DocString -> 'd'String
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
    lower_range_prec = 0;
    // Multiline parsing cannot be zeroes in there as this might be called
    // in multiline declaration
    assert(multi_line_parsing >= 0 && "Got out of multiline structures more than into them?");
    bool no_end_needed = false;

    // Skip random new lines and ;
    skip_ends();

    // This could be uncommented to speed up parsing and error could be just in note
    parser_assert(!bind_docstring(), create_diag(diags::DOC_STRING_NOT_AT_START));

    // end of file returns End IR
    if (match(TokenType::END_OF_FILE)) {
        return new EndOfFile(curr_src_info());
    }

    if (check(TokenType::ERROR_TOKEN)) {
        auto err_tok = dynamic_cast<ErrorToken *>(advance());
        auto str_msg = error::format_error(diags::Diagnostic(err_tok, scanner));
        SourceInfo srci;
        throw new Raise(new StringLiteral(str_msg, srci), srci);
    }

    // Consume annotation and tie inner ones to parents and outter ones save
    // for next IR
    Annotation *annot = annotation();
    bool was_inner_anot = false;
    while(annot) {
        LOGMAX("Parsing annotation: " << *annot);
        if (annot->is_inner()) {
            assert(!parents.empty() && "No top level IR?");
            auto parent = parents.back();
            parser_assert(parent->can_be_annotated(), create_diag(diags::CANNOT_BE_ANNOTATED, parent->get_name().c_str()));
            // For module we want to output the annotation as IR
            if (parents.size() == 1) {
                LOGMAX("Outputting module annotation");
                return annot;
            } else {
                parents.back()->add_annotation(annot);
            }
            was_inner_anot = true;
        }
        else {
            outter_annots.push_back(annot);
        }
        skip_ends();
        annot = annotation();
    }

    // import
    if (match(TokenType::IMPORT)) {
        auto importsrci = curr_src_info();
        std::vector<ir::Expression *> names;
        std::vector<ustring> aliases;
        ++lower_range_prec;

        Expression *name = nullptr;
        do {
            if (name)
                skip_nls();
            name = call_access_subs(true);
            parser_assert(name, create_diag(diags::MEMBER_OR_ID_EXPECTED));
            names.push_back(name);
            // We always call get_last_id as it checks that the name is ID or Scope
            ustring alias = get_last_id(name);
            if (match(TokenType::AS)) {
                auto alias_tok = expect(TokenType::ID, create_diag(diags::ID_EXPECTED));
                alias = alias_tok->get_value();
            }
            aliases.push_back(alias);
        } while (match(TokenType::COMMA) && name);

        --lower_range_prec;
        importsrci.update_ends(curr_src_info());
        decl = new Import(names, aliases, importsrci);
    }

    // if
    else if (match(TokenType::IF)) {
        auto ifsrci = curr_src_info();
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
            auto elsesrci = curr_src_info();
            auto elsebody = body();
            elsesrci.update_ends(curr_src_info());
            ifsrci.update_ends(curr_src_info());
            decl = new If(cond, ifbody, new Else(elsebody, elsesrci), ifsrci);
        }
        else {
            ifsrci.update_ends(curr_src_info());
            decl = new If(cond, ifbody, nullptr, ifsrci);
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
        auto whilesrci = curr_src_info();
        expect(TokenType::LEFT_PAREN, create_diag(diags::WHILE_REQUIRES_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        auto whbody = body();
        whilesrci.update_ends(curr_src_info());
        decl = new While(cond, whbody, whilesrci);
        no_end_needed = true;
    }
    else if (match(TokenType::DO)) {
        auto dosrc = curr_src_info();
        auto whbody = body();
        skip_nls();
        expect(TokenType::WHILE, create_diag(diags::NO_WHILE_AFTER_DO));
        expect(TokenType::LEFT_PAREN, create_diag(diags::WHILE_REQUIRES_PARENTH));
        auto cond = expression();
        parser_assert(cond, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        dosrc.update_ends(curr_src_info());
        decl = new DoWhile(cond, whbody, dosrc);
    }

    // for
    else if (match(TokenType::FOR)) {
        auto forsrci = curr_src_info();
        expect(TokenType::LEFT_PAREN, create_diag(diags::FOR_REQUIRES_PARENTH));
        ++lower_range_prec;
        int rest_index = -1;
        if (match(TokenType::THREE_DOTS)) {
            rest_index = 0;
        }
        auto iterator = expression();
        --lower_range_prec;
        if (match(TokenType::COMMA)) {
            std::vector<ir::Expression *> vars = {iterator};
            Expression *expr = nullptr;
            ++lower_range_prec;
            do {
                skip_nls();
                if (match(TokenType::THREE_DOTS)) {
                    parser_assert(rest_index == -1, create_diag(diags::MULTIPLE_3DOT_MULTIVAR));
                    rest_index = static_cast<int>(vars.size());
                }
                expr = expression(true);
                if (expr) {
                    parser_assert(is_id_or_member(expr), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
                    vars.push_back(expr);
                }
            } while (match(TokenType::COMMA) && expr);
            iterator = new Multivar(vars, rest_index, curr_src_info());
        }
        parser_assert(is_id_or_member(iterator) || isa<Multivar>(iterator), create_diag(diags::MEMBER_OR_ID_EXPECTED));
        parser_assert(iterator, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::COLON, create_diag(diags::FOR_MISSING_COLON));
        auto collection = expression();
        parser_assert(collection, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        auto frbody = body();
        forsrci.update_ends(curr_src_info());
        decl = new ForLoop(iterator, collection, frbody, forsrci);
        no_end_needed = true;
    }

    // switch
    else if (match(TokenType::SWITCH)) {
        auto swtsrci = curr_src_info();
        expect(TokenType::LEFT_PAREN, create_diag(diags::SWITCH_REQUIRES_PARENTH));
        auto val = expression();
        parser_assert(val, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        ++multi_line_parsing;
        auto sw_cases = cases();
        no_end_needed = true;
        --multi_line_parsing;
        swtsrci.update_ends(curr_src_info());
        decl = new Switch(val, sw_cases, swtsrci);
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
        SourceInfo trysrci = curr_src_info();
        auto trbody = body();
        skip_nls();
        std::vector<Catch *> catches;
        while (match(TokenType::CATCH)) {
            auto catchsrci = curr_src_info();
            expect(TokenType::LEFT_PAREN, create_diag(diags::CATCH_REQUIRES_PARENTH));
            auto arg = argument();
            parser_assert(arg, create_diag(diags::EXPR_EXPECTED));
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
            auto ctbody = body();
            catchsrci.update_ends(curr_src_info());
            catches.push_back(new Catch(arg, ctbody, catchsrci));
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
            auto finsrci = curr_src_info();
            auto fnbody = body();
            finsrci.update_ends(curr_src_info());
            finallyStmt = new Finally(fnbody, finsrci);
        }
        trysrci.update_ends(curr_src_info());
        decl = new Try(catches, trbody, finallyStmt, trysrci);
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
        decl = new Assert(args[0], args.size() == 2 ? args[1] : nullptr, curr_src_info());
    }
    else if (match(TokenType::RAISE)) {
        auto exc = expression();
        parser_assert(exc, create_diag(diags::EXPR_EXPECTED));
        decl = new Raise(exc, curr_src_info());
    }
    else if (match(TokenType::RETURN)) { // TODO: Check if inside of a function
        auto exc = expression();
        if (!exc) {
            decl = new Return(new NilLiteral(curr_src_info()), curr_src_info());
        }
        else {
            decl = new Return(exc, curr_src_info());
        }
    }
    // break / continue
    else if (match(TokenType::BREAK)) {
        decl = new Break(curr_src_info());
    }
    else if (match(TokenType::CONTINUE)) {
        decl = new Continue(curr_src_info());
    }
    // enum
    else if (match(TokenType::ENUM)) {
        SourceInfo enumsrci = curr_src_info();
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
        enumsrci.update_ends(curr_src_info());
        decl = new Enum(name->get_value(), values, enumsrci);
        no_end_needed = true;
    }

    // class
    else if (match(TokenType::CLASS)) {
        ++multi_line_parsing;
        if (check(TokenType::ID)) {
            auto name = advance();
            std::vector<Expression *> clparents;
            if (match(TokenType::COLON)) {
                clparents = expr_list(true);
                parser_assert(!clparents.empty(), create_diag(diags::PARENT_LIST_EXPECTED));
            }
            // We need to store annotation so that inner IRs don't consume them
            std::list<ir::Annotation *> cl_annts;
            cl_annts.insert(cl_annts.end(), std::make_move_iterator(outter_annots.begin()), std::make_move_iterator(outter_annots.end()));
            outter_annots.clear();
            auto cldecl = new Class(name->get_value(), clparents, curr_src_info());
            parents.push_back(cldecl);
            auto clbody = block();
            parents.pop_back();
            cldecl->set_body(clbody);
            decl = cldecl;
            decl->get_src_info().update_ends(curr_src_info());
            // Assign outter annotations
            for (auto ann : cl_annts) {
                decl->add_annotation(ann);
            }
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
            auto spcdecl = new Space(name->get_value(), curr_src_info());
            parents.push_back(spcdecl);
            auto stbody = block();
            parents.pop_back();
            spcdecl->set_body(stbody);
            decl = spcdecl;
            decl->get_src_info().update_ends(curr_src_info());
            no_end_needed = true;
        }
        else if (check(TokenType::LEFT_CURLY)) {
            auto spcdecl = new Space("", curr_src_info());
            parents.push_back(spcdecl);
            auto stbody = block();
            parents.pop_back();
            spcdecl->set_body(stbody);
            decl = spcdecl;
            decl->get_src_info().update_ends(curr_src_info());
            no_end_needed = true;
        }
        else {
            parser_error(create_diag(diags::MISSING_SPACE_BODY));
        }
        --multi_line_parsing;
    }

    // fun / constructor / lambdas
    else if (match(TokenType::FUN)) {
        ustring name = "";
        if (check(TokenType::ID)) {
            auto id = advance();
            name = id->get_value();
        }
        expect(TokenType::LEFT_PAREN, create_diag(diags::FUN_REQUIRES_PARENTH));
        // Check operator function
        auto op_name = operator_name();
        if (op_name) {
            name = op_name->get_op().as_string();
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
            expect(TokenType::LEFT_PAREN, create_diag(diags::FUN_REQUIRES_PARENTH));
            delete op_name;
        }
        std::vector<Argument *> args;
        if (!match(TokenType::RIGHT_PAREN)) {
            args = arg_list();
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        }
        skip_nls();
        if (check(TokenType::LEFT_CURLY)) {
            decl = new Function(name, args, curr_src_info());
            // Assign outter annotations
            for (auto ann : outter_annots) {
                decl->add_annotation(ann);
            }
            outter_annots.clear();
            parents.push_back(decl);
            auto fnbody = block();
            parents.pop_back();
            if (name.empty()) {
                parser_error(create_diag(diags::ANONYMOUS_FUN));
            }
            auto fn = dyn_cast<Function>(decl);
            fn->set_body(fnbody);
            fn->get_src_info().update_ends(curr_src_info());
        }
        else if (match(TokenType::SET)) {
            auto lmbody = expression();
            parser_assert(lmbody, create_diag(diags::EXPR_EXPECTED));
            decl = new Lambda(name, args, lmbody, curr_src_info());
            // Assign outter annotations
            for (auto ann : outter_annots) {
                decl->add_annotation(ann);
            }
            outter_annots.clear();
        }
        else {
            parser_error(create_diag(diags::MISSING_FUN_BODY));
        }
    }

    // expression
    else if (auto expr = expression(true)) {
        decl = expr;
    }

    if (!decl && !outter_annots.empty())
        parser_error(create_diag(diags::DANGLING_ANNOTATION));

    if (!decl && was_inner_anot) {
        return nullptr;
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
    
    assert(decl && "Nothing parsed and no raise?");
    LOGMAX("Parsed declaration " << *decl);
    
    parser_assert(outter_annots.empty(), create_diag(diags::CANNOT_BE_ANNOTATED, decl->get_name().c_str()));

    return decl;
}

bool Parser::bind_docstring() {
    // Skip random new lines, ; and white spaces
    skip_ends_and_ws();

    // Extracting any doc strings
    bool bound = false;
    while (check(TokenType::ID)) {
        auto tt = peek(0);
        if (tt->get_value() == "d" && peek(1)->get_type() == TokenType::STRING) {
            advance();
            auto val = advance();
            assert(!parents.empty() && "No top level IR?");
            auto parent = parents.back();
            parser_assert(parent->can_be_documented(), create_diag(diags::CANNOT_BE_DOCUMENTED, parent->get_name().c_str()));
            if (auto mdl = dyn_cast<Module>(parent)) {
                parser_assert(mdl->size() == 0, create_diag(diags::DOC_STRING_NOT_AT_START));
            }
            parent->add_documentation(val->get_value());
            skip_ends();
            bound = true;
        } else {
            break;
        }
    }
    return bound;
}

std::list<ir::IR *> Parser::block() {
    skip_nls();
    expect(TokenType::LEFT_CURLY, create_diag(diags::MISSING_BLOCK));
    skip_nls();
    std::list<ir::IR *> decls;

    IR *decl = nullptr;
    bind_docstring();
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
        decls.push_back(new Case(vals, swbody, default_case, curr_src_info()));
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
 * 
 * List ->
 * Dict ->
 * ```
 */
Expression *Parser::expression(bool allow_set) {
    Expression *expr = silent();
    if (!allow_set && expr) {
        if (auto be = dyn_cast<BinaryExpr>(expr)) {
            parser_assert(!ir::is_set_op(be->get_op()), create_diag(diags::SET_NOT_ALLOWED));
        }
    }
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
        return new UnaryExpr(expr, Operator(OperatorKind::OP_SILENT), curr_src_info());
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
        expr = new BinaryExpr(expr, right,  Operator(OperatorKind::OP_SET), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
    }

    return expr;
}

Expression *Parser::unpack() {
    if (match(TokenType::UNPACK)) {
        auto expr = ternary_if();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_UNPACK), curr_src_info());
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
        return new TernaryIf(expr, val_true, val_false, curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
    }

    return expr;
}

Expression *Parser::op_not() {
    if (match(TokenType::NOT)) {
        auto expr = op_not(); // Right associative
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_NOT), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
    }

    return expr;
}

Expression *Parser::membership() {
    Expression *expr = range();

    if (match(TokenType::IN)) {
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "in"));
        auto right = range(); // Chaining in is not allowed, parenthesis have to be used
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        return new BinaryExpr(expr, right, Operator(OperatorKind::OP_IN), curr_src_info());
    }

    return expr;
}

Expression *Parser::list_of_vars(Expression *first, Expression *second, int rest_index) {
    parser_assert(is_id_or_member(first), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
    parser_assert(is_id_or_member(second), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
    std::vector<ir::Expression *> vars = {first, second};
    Expression *expr = nullptr;
    ++lower_range_prec;
    bool found_eq = false;
    do {
        skip_nls();
        if (match(TokenType::THREE_DOTS)) {
            parser_assert(rest_index == -1, create_diag(diags::MULTIPLE_3DOT_MULTIVAR));
            rest_index = static_cast<int>(vars.size());
        }
        expr = expression(true);
        if (expr) {
            if (auto be = dyn_cast<BinaryExpr>(expr)) {
                if (be->get_op().get_kind() == OperatorKind::OP_SET) {
                    parser_assert(is_id_or_member(be->get_left()), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
                    vars.push_back(be->get_left());
                    expr = be->get_right();
                    be->set_left(nullptr);
                    be->set_right(nullptr);
                    found_eq = true;
                    break;
                }
                else {
                    parser_error(create_diag(diags::SET_EXPECTED_FOR_MULTIVAL));
                }
            }
            else {
                parser_assert(is_id_or_member(expr), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
                vars.push_back(expr);
            }
        }
    } while (match(TokenType::COMMA) && expr);
    parser_assert(found_eq, create_diag(diags::SET_EXPECTED_FOR_MULTIVAL));
    --lower_range_prec;
    return new BinaryExpr(new Multivar(vars, rest_index, curr_src_info()), expr, Operator(OperatorKind::OP_SET), curr_src_info());
}

Expression *Parser::range() {
    Expression *expr = concatenation();

    // expr..end
    // TODO: Raise a warning if in fun args and not in parenthesis
    if (match(TokenType::RANGE)) {
        parser_assert(expr, create_diag(diags::NO_LHS_IN_RANGE));
        auto end = concatenation();
        parser_assert(end, create_diag(diags::EXPR_EXPECTED));
        return new Range(expr, end, nullptr, curr_src_info());
    }
    // expr,second..end
    else if (!lower_range_prec && match(TokenType::COMMA)) {
        parser_assert(expr, create_diag(diags::NO_LHS_IN_RANGE));
        int rest_index = -1;
        if (match(TokenType::THREE_DOTS)) {
            rest_index = 1;
        }
        auto second = concatenation();
        parser_assert(second, create_diag(diags::EXPR_EXPECTED));
        // List of value to assing to
        if (match(TokenType::COMMA)) {
            return list_of_vars(expr, second, rest_index);
        }
        else if (match(TokenType::SET)) {
            std::vector<ir::Expression *> vars = {expr, second};
            parser_assert(is_id_or_member(expr), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
            parser_assert(is_id_or_member(second), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
            auto res = concatenation();
            return new BinaryExpr(new Multivar(vars, rest_index, curr_src_info()), res, Operator(OperatorKind::OP_SET), curr_src_info());
        }
        expect(TokenType::RANGE, create_diag(diags::RANGE_EXPECTED));
        auto end = concatenation();
        parser_assert(second, create_diag(diags::EXPR_EXPECTED));
        return new Range(expr, end, second, curr_src_info());
    }

    return expr;
}

Expression *Parser::concatenation() {
    Expression *expr = add_sub();

    while (match(TokenType::CONCAT)) {
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "++"));
        auto right = add_sub();
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, Operator(OperatorKind::OP_CONCAT), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
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
        expr = new BinaryExpr(expr, right, token2operator(op->get_type()), curr_src_info());
    }

    return expr;
}

Expression *Parser::exponentiation() {
    Expression *expr = unary_plus_minus();

    while (match(TokenType::EXP)) {
        parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "^"));
        auto right = exponentiation(); // Right associative, so call itself
        parser_assert(right, create_diag(diags::EXPR_EXPECTED));
        expr = new BinaryExpr(expr, right, Operator(OperatorKind::OP_EXP), curr_src_info());
    }

    return expr;
}

Expression *Parser::unary_plus_minus() {
    if (match(TokenType::MINUS)) {
        auto expr = unary_plus_minus(); // Right associative
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(expr, Operator(OperatorKind::OP_MINUS), curr_src_info());
    }
    else if (match(TokenType::PLUS)) {
        auto expr = unary_plus_minus();
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        return expr;
    }

    return call_access_subs();
}


std::vector<ir::Expression *> Parser::expr_list(bool only_scope_or_id, bool allow_set) {
    std::vector<ir::Expression *> args;
    // Setting this to true will indicate to not give precedence to range over
    // another argument
    ++lower_range_prec;

    Expression *expr = nullptr;
    do {
        skip_nls();
        expr = expression(allow_set);
        if (expr) {
            if (only_scope_or_id)
                parser_assert(is_id_or_member(expr), create_diag(diags::MEMBER_OR_ID_EXPECTED));
            args.push_back(expr);
        }
    } while (match(TokenType::COMMA) && expr);

    --lower_range_prec;

    return args;
}

std::vector<ir::Argument *> Parser::arg_list() {
    std::vector<ir::Argument *> args;
    ++lower_range_prec;

    Argument *arg = nullptr;
    bool vararg = false;
    do {
        skip_nls();
        if (match(TokenType::THREE_DOTS)) {
            parser_assert(!vararg, create_diag(diags::MULTIPLE_VARARGS));
            auto id = expect(TokenType::ID, create_diag(diags::ID_EXPECTED));
            parser_assert(!check({TokenType::COLON, TokenType::SET}), create_diag(diags::NOT_PURE_VARARG));
            arg = new Argument(id->get_value(), curr_src_info());
            vararg = true;
        }
        else {
            arg = argument(true);
        }
        assert(arg && "Argument not returned");
        args.push_back(arg);
    } while (match(TokenType::COMMA));

    --lower_range_prec;
    return args;
}

Expression *Parser::call_access_subs(bool allow_star) {
    Expression *expr = note();

    while(check({TokenType::LEFT_PAREN, TokenType::DOT, TokenType::LEFT_SQUARE})) {
        if (match(TokenType::LEFT_PAREN)) {
            // This assert should never be raised as ( would be matched in constant
            parser_assert(expr, create_diag(diags::BIN_OP_REQUIRES_LHS, "()"));
            // All exprs and allow set for named function params
            auto args = expr_list(false, true);
            skip_nls();
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
            expr = new Call(expr, args, curr_src_info());
        }
        else if (match(TokenType::DOT)) {
            parser_assert(expr, create_diag(diags::NO_LHS_IN_ACCESS));
            if (match(TokenType::MUL)) {
                parser_assert(allow_star, create_diag(diags::STAR_MEMBER_OUTSIDE_IMPORT)); 
                parser_assert(expr, create_diag(diags::STAR_IMPORT_GLOBAL));
                // Return since there cannot be anything else after this
                return new BinaryExpr(expr, new AllSymbols(curr_src_info()), Operator(OperatorKind::OP_ACCESS), curr_src_info());
            }
            else {
                auto elem = note();
                parser_assert(elem, create_diag(diags::EXPR_EXPECTED));
                expr = new BinaryExpr(expr, elem, Operator(OperatorKind::OP_ACCESS), curr_src_info());
            }
        }
        else if (match(TokenType::LEFT_SQUARE)) {
            parser_assert(expr, create_diag(diags::NO_LHS_IN_SUBSCRIPT));
            auto index = ternary_if();
            parser_assert(index, create_diag(diags::EXPR_EXPECTED));
            expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
            expr = new BinaryExpr(expr, index, Operator(OperatorKind::OP_SUBSC), curr_src_info());
        }
    }
    return expr;
}

ir::Expression *Parser::note() {
    Expression *expr = scope();

    if (check_ws(TokenType::STRING)) {
        // expr should never be nullptr as string would be parsed in constant
        assert(expr && "Somehow string was trying to be parsed as a note without prefix");
        auto val = advance();
        // We don't unescape the string as the note might use its own escape
        // sequences and unescape has to be called by the formatter in that case
        auto prefix = dyn_cast<Variable>(expr);
        parser_assert(prefix, create_diag(diags::INVALID_NOTE_PREFIX));
        if (prefix->get_name() == "r") {
            // rString
            return new StringLiteral(val->get_value(), curr_src_info());
        }
        parser_assert(prefix->get_name() != "d", create_diag(diags::DOC_STRING_AS_EXPR));
        return new Note(prefix->get_name(), new StringLiteral(val->get_value(), curr_src_info()), curr_src_info());
    }

    return expr;
}

Expression *Parser::scope() {
    if (match(TokenType::SCOPE)) {
        auto elem = constant();
        parser_assert(elem, create_diag(diags::EXPR_EXPECTED));
        return new UnaryExpr(elem, Operator(OperatorKind::OP_SCOPE), curr_src_info());
    }

    return constant();
}

ir::OperatorLiteral *Parser::operator_name() {
    // []
    if (match(TokenType::LEFT_SQUARE)) {
        if (match(TokenType::RIGHT_SQUARE)) {
            return new OperatorLiteral(Operator(OperatorKind::OP_SUBSC), curr_src_info());
        } else {
            put_back();
        }
    }
    if (check({TokenType::CONCAT, TokenType::EXP, TokenType::PLUS,
          TokenType::MINUS, TokenType::DIV, TokenType::MUL,
          TokenType::MOD, TokenType::EQ, TokenType::NEQ,
          TokenType::BT, TokenType::LT, TokenType::BEQ, TokenType::LEQ,
          TokenType::SHORT_C_AND, TokenType::SHORT_C_OR, TokenType::AND,
          TokenType::OR, TokenType::NOT, TokenType::XOR, TokenType::IN})) {
        auto op = token2operator(advance()->get_type());
        return new OperatorLiteral(op, curr_src_info());
    }
    return nullptr;
}

Expression *Parser::fstring(FStringToken *fstr) {
    Expression *retv = nullptr;
    std::string txt;

    this->spill_tokens(fstr->get_tokens());

    do {
        if (match(TokenType::LEFT_CURLY)) {
            auto expr = expression();
            parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
            if (retv) {
                retv = new BinaryExpr(retv, expr, Operator(OperatorKind::OP_CONCAT), curr_src_info());
            } else {
                retv = expr;
            }
            expect(TokenType::RIGHT_CURLY, create_diag(diags::EXPECTED_CLOSE_FSTRING_EXPR));
        } else if (!check(TokenType::QUOTE)){
            auto c = advance();
            txt = c->get_value();
            if (retv) {
                retv = new BinaryExpr(retv, new StringLiteral(unescapeString(txt), curr_src_info()), Operator(OperatorKind::OP_CONCAT), curr_src_info());
            } else {
                retv = new StringLiteral(unescapeString(txt), curr_src_info());
            }
            txt = "";
        }
    } while (!match(TokenType::QUOTE));
    // FString has to end with single " no matter if it is multiline or not
    if (!retv)
        return new StringLiteral("", curr_src_info());
    return retv;
}

Expression *Parser::constant() {
    if (match(TokenType::LEFT_PAREN)) {
        bool prev_fun_args_state = lower_range_prec;
        lower_range_prec = 0; // This will allow for range in function call
        Expression *expr = operator_name();
        if (!expr)
            expr = ternary_if();
        else {
            // There might be (+) or (+1)
            if (!check(TokenType::RIGHT_PAREN)) {
                put_back();
                expr = ternary_if();
            }
        }
        parser_assert(expr, create_diag(diags::EXPR_EXPECTED));
        expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        lower_range_prec = prev_fun_args_state;
        return expr;
    }
    else if (match(TokenType::NON_LOCAL)) {
        auto id = expect(TokenType::ID, create_diag(diags::ID_EXPECTED));
        return new Variable(id->get_value(), curr_src_info(), true);
    }
    else if (check(TokenType::ID)) {
        auto id = advance();
        return new Variable(id->get_value(), curr_src_info());
    }
    else if (match(TokenType::THREE_DOTS)) {
        ++lower_range_prec;
        auto first = expression();
        parser_assert(first, create_diag(diags::ID_EXPECTED));
        expect(TokenType::COMMA, create_diag(diags::COMMA_FOR_MULTIVAR_EXPECTED));
        parser_assert(!match(TokenType::THREE_DOTS), create_diag(diags::MULTIPLE_3DOT_MULTIVAR));
        auto second = concatenation();
        parser_assert(second, create_diag(diags::ID_EXPECTED));
        --lower_range_prec;
        // List of value to assing to
        if (match(TokenType::COMMA)) {
            return list_of_vars(first, second, 0);
        }
        expect(TokenType::SET, create_diag(diags::SET_EXPECTED_FOR_MULTIVAL));
        std::vector<ir::Expression *> vars = {first, second};
        parser_assert(is_id_or_member(first), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
        parser_assert(is_id_or_member(second), create_diag(diags::EXPR_CANNOT_BE_ASSIGN_TO));
        auto res = concatenation();
        return new BinaryExpr(new Multivar(vars, 0, curr_src_info()), res, Operator(OperatorKind::OP_SET), curr_src_info());
    }
    else if (check(TokenType::INT)) {
        auto val = advance();
        // The value was parsed and checked, so no need to check for
        // correct conversion, but we need to check for under/overflow
        char *end;
        errno = 0;
        opcode::IntConst ival = std::strtol(val->get_value().c_str(), &end, 10);
        if (errno == ERANGE) {
            error::warning(diags::Diagnostic(true, this->src_file, curr_src_info(), scanner, diags::WarningID::INT_CANNOT_FIT, val->get_value().c_str()));
            // We set the long to max value as in C++ it is UB
            ival = std::numeric_limits<opcode::IntConst>::max();
            errno = 0;
        }
        //assert(*end == '\0' && "std::strtol fail");
        return new IntLiteral(atol(val->get_value().c_str()), curr_src_info());
    }
    else if (check(TokenType::FLOAT)) {
        auto val = advance();
        try {
            return new FloatLiteral(std::stod(val->get_value()), curr_src_info());
#ifndef NDEBUG 
        } catch (const std::invalid_argument &e) {
            assert(false && "std::stod failed");
#endif
        } catch (const std::out_of_range& e) {
            error::warning(diags::Diagnostic(true, this->src_file, curr_src_info(), scanner, diags::WarningID::FLOAT_CANNOT_FIT, val->get_value().c_str()));
            // Handle overflow/underflow
            auto str = val->get_value();
            std::size_t e_pos = str.find_first_of("eE");
            if (e_pos != std::string::npos) {
                try {
                    int exponent = std::stoi(str.substr(e_pos + 1));
                    if (exponent > 0) {
                        return new FloatLiteral(std::numeric_limits<opcode::FloatConst>::infinity(), curr_src_info());
                    } else {
                        return new FloatLiteral(0.0, curr_src_info());  // Underflow
                    }
                } catch (...) {
                    // Malformed exponent, fallback
                }
            }

            // Fallback: if no exponent or malformed, assume overflow
            return new FloatLiteral(std::numeric_limits<opcode::FloatConst>::infinity(), curr_src_info());
        }
    }
    else if (check(TokenType::STRING)) {
        auto val = advance();
        return new StringLiteral(unescapeString(val->get_value()), curr_src_info());
    }
    else if (check(TokenType::FSTRING)) {
        auto val = advance();
        FStringToken *fstrtok = dynamic_cast<FStringToken *>(val);
        return fstring(fstrtok);
    }
    else if (match(TokenType::TRUE)) {
        return new BoolLiteral(true, curr_src_info());
    }
    else if (match(TokenType::FALSE)) {
        return new BoolLiteral(false, curr_src_info());
    }
    else if (match(TokenType::NIL)) {
        return new NilLiteral(curr_src_info());
    }
    else if (match(TokenType::THIS)) {
        return new ThisLiteral(curr_src_info());
    }
    else if (match(TokenType::SUPER)) {
        return new SuperLiteral(curr_src_info());
    }
    // lambda
    else if (match(TokenType::FUN)) {
        ustring name = "";
        if (check(TokenType::ID)) {
            auto id = advance();
            name = id->get_value();
        }
        expect(TokenType::LEFT_PAREN, create_diag(diags::FUN_REQUIRES_PARENTH));
        std::vector<Argument *> args;
        if (!match(TokenType::RIGHT_PAREN)) {
            args = arg_list();
            expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
        }
        skip_nls();
        parser_assert(!check(TokenType::LEFT_CURLY), create_diag(diags::LAMBDA_WITH_BODY));
        expect(TokenType::SET, create_diag(diags::SET_EXPECTED));
        auto lmbody = expression();
        parser_assert(lmbody, create_diag(diags::EXPR_EXPECTED));
        return new Lambda(name, args, lmbody, curr_src_info());
    }
    // list
    else if (match(TokenType::LEFT_SQUARE)) {
        ++lower_range_prec;
        std::vector<ir::Expression *> vals;
        skip_nls();
        // Lets extract first expr and then check if this is a list comprehension
        auto first = expression();
        if (first) {
            if (check({TokenType::COLON, TokenType::IF})) {
                skip_nls();
                Expression *condition = nullptr;
                Expression *else_result = nullptr;
                if (check(TokenType::IF)) {
                    advance();
                    expect(TokenType::LEFT_PAREN, create_diag(diags::IF_REQUIRES_PARENTH));
                    condition = expression();
                    parser_assert(condition, create_diag(diags::EXPR_EXPECTED));
                    expect(TokenType::RIGHT_PAREN, create_diag(diags::MISSING_RIGHT_PAREN));
                    if (match(TokenType::ELSE)) {
                        else_result = expression();
                        parser_assert(else_result, create_diag(diags::EXPR_EXPECTED));
                    }
                }
                expect(TokenType::COLON, create_diag(diags::COLON_EXPECTED));
                skip_nls();
                auto assignments = expr_list(false, true);
                // Check if all values are assignments
                for (auto a : assignments) {
                    auto be = dyn_cast<BinaryExpr>(a);
                    // TODO: Emit error recommending putting range in parenthesis if 
                    parser_assert(be && be->get_op().get_kind() == OperatorKind::OP_SET, create_diag(diags::LIST_COMP_NOT_ASSIGN));
                }
                skip_nls();
                expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
                --lower_range_prec;
                return new List(first, assignments, condition, else_result, curr_src_info());
            }
            else if (match(TokenType::COMMA)){
                skip_nls();
                vals = expr_list();
                vals.insert(vals.begin(), first);
            }
            else if (check(TokenType::RIGHT_SQUARE)) {
                vals.push_back(first);
            }
        }
        skip_nls();
        expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
        --lower_range_prec;
        return new List(vals, curr_src_info());
    }
    // dict
    else if (match(TokenType::LEFT_CURLY)) {
        std::vector<ir::Expression *> keys;
        std::vector<ir::Expression *> vals;
        if (match(TokenType::COLON)) {
            expect(TokenType::RIGHT_CURLY, create_diag(diags::MISSING_RIGHT_CURLY));
            return new Dict(keys, vals, curr_src_info());
        }
        ++lower_range_prec;

        Expression *expr = nullptr;
        do {
            skip_nls();
            expr = expression();
            if (expr) {
                keys.push_back(expr);
                expect(TokenType::COLON, create_diag(diags::DICT_NO_COLON));
                auto val = expression();
                parser_assert(val, create_diag(diags::EXPR_EXPECTED));
                vals.push_back(val);
            }
        } while (match(TokenType::COMMA) && expr);

        --lower_range_prec;
        skip_nls();
        expect(TokenType::RIGHT_CURLY, create_diag(diags::MISSING_RIGHT_CURLY));
        parser_assert(!keys.empty(), create_diag(diags::EMPTY_DICT_WITHOUT_COLON));
        return new Dict(keys, vals, curr_src_info());
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
                ++lower_range_prec;
                Expression *expr = nullptr;
                do {
                    skip_nls();
                    expr = expression();
                    if (expr) {
                        parser_assert(is_id_or_member(expr), create_diag(diags::MEMBER_OR_ID_EXPECTED));
                        types.push_back(expr);
                    }
                } while (match(TokenType::COMMA) && expr);
                parser_assert(!types.empty(), create_diag(diags::TYPE_EXPECTED));
                expect(TokenType::RIGHT_SQUARE, create_diag(diags::MISSING_RIGHT_SQUARE));
                --lower_range_prec;
            }
            else {
                auto type = call_access_subs();
                parser_assert(type, create_diag(diags::TYPE_EXPECTED));
                parser_assert(is_id_or_member(type), create_diag(diags::MEMBER_OR_ID_EXPECTED));
                types.push_back(type);
            }
        }
        if (match(TokenType::SET)) {
            parser_assert(allow_default_value, create_diag(diags::DEFAULT_NOT_ALLOWED));
            default_value = expression();
            parser_assert(default_value, create_diag(diags::EXPR_EXPECTED));
        }
        return new Argument(id->get_value(), types, default_value, curr_src_info());
    }
    else {
        parser_error(create_diag(diags::INCORRECT_ARGUMENT));
    }
    return nullptr;
}

static std::string unicode2UTF8(char16_t codepoint) {
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
    return conv.to_bytes(codepoint);
}

static std::string unicode2UTF8(char32_t codepoint) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(codepoint);
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
                case 'X': {
                    parser_assert(i+2 < str.length(), create_diag(diags::SHORT_HEX_ESC_SEQ));
                    auto v = str.substr(i+1, 2);
                    char *end;
                    errno = 0;
                    auto ival = std::strtol(v.c_str(), &end, 16);
                    parser_assert(*end == '\0' && errno == 0, create_diag(diags::INCORRECT_HEX_ESC_SEQ, v.c_str()));
                    res << static_cast<char>(ival);
                    i+=2;
                }
                break;
                case 'q':
                case 'Q': {
                    parser_assert(i+3 < str.length(), create_diag(diags::SHORT_OCT_ESC_SEQ));
                    auto v = str.substr(i+1, 3);
                    char *end;
                    errno = 0;
                    auto ival = std::strtol(v.c_str(), &end, 8);
                    parser_assert(*end == '\0' && errno == 0, create_diag(diags::INCORRECT_OCT_ESC_SEQ, v.c_str()));
                    res << static_cast<char>(ival);
                    i+=3;
                }
                break;
                case 'u': {
                    parser_assert(i+4 < str.length(), create_diag(diags::SHORT_UNICODE16_ESC_SEQ));
                    auto v = str.substr(i+1, 4);
                    char *end;
                    errno = 0;
                    auto ival = std::strtol(v.c_str(), &end, 16);
                    parser_assert(*end == '\0' && errno == 0, create_diag(diags::INCORRECT_UNICODE16_ESC_SEQ, v.c_str()));
                    res << unicode2UTF8(static_cast<char16_t>(ival));
                    i+=4;
                }
                break;
                case 'U': {
                    parser_assert(i+8 < str.length(), create_diag(diags::SHORT_UNICODE32_ESC_SEQ));
                    auto v = str.substr(i+1, 8);
                    char *end;
                    errno = 0;
                    auto ival = std::strtol(v.c_str(), &end, 16);
                    parser_assert(*end == '\0' && errno == 0, create_diag(diags::INCORRECT_UNICODE32_ESC_SEQ, v.c_str()));
                    res << unicode2UTF8(static_cast<char32_t>(ival));
                    i+=8;
                }
                break;
                case 'N':
                    parser_error(create_diag(diags::UNIMPLEMENTED_SYNTAX_FEATURE, "Named unicode escape sequences ('\\N{name}') are not yet implemented"));
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