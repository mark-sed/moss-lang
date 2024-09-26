/**
 * @file ir.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Internal representation of moss constructs.
 * This IR is then lowered into bytecode.
 */

#ifndef _IR_HPP_
#define _IR_HPP_

#include "source.hpp"
#include "os_interface.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <cassert>
#include <list>
#include <vector>

#include <sstream>

namespace moss {

namespace ir {

enum IRType {
    CONSTRUCT,
    EXPRESSION,
    STATEMENT,

    MODULE,
    SPACE,
    CLASS,
    FUNCTION,
    IF,
    ELSE,
    SWITCH,
    CASE,
    TRY,
    CATCH,
    FINALLY,
    WHILE,
    DO_WHILE,
    FOR_LOOP,
    ENUM,

    IMPORT,
    ASSERT,
    RAISE,
    RETURN,
    BREAK,
    CONTINUE,
    ANNOTATION,

    BINARY_EXPR, // Start of Expresion IDs -- Add any new to dyn_cast bellow!
    UNARY_EXPR,
    VARIABLE,
    ARGUMENT,
    NOTE,
    TERNARY_IF,
    RANGE,
    CALL,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    NIL_LITERAL, // End of Expression IDs

    END_OF_FILE,
};

class Annotation;

/** Base class for any IR value */
class IR {
protected:
    IRType ir_type;
    ustring name;
    std::list<Annotation *> annotations;

    IR(IRType ir_type, ustring name) : ir_type(ir_type), name(name) {}
public:
    virtual ~IR() {}

    IRType get_type() { return ir_type; }
    ustring get_name() { return name; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<IR: " << name << ">";
        return os;
    }

    virtual void add_annot(Annotation *ann) {
        annotations.push_back(ann);
    }
};

inline std::ostream& operator<< (std::ostream& os, IR &ir) {
    return ir.debug(os);
}

bool can_be_annotated(IR *decl);

/** 
 * @brief IRs that are constructs
 * Constructs cannot appear in an expression and have certain control flow
 * and body.
 */
class Construct : public IR {
protected:
    std::list<IR *> body;
    Construct(IRType ir_type, ustring name) : IR(ir_type, name), body() {}
    Construct(IRType ir_type, ustring name, std::list<IR *> body) : IR(ir_type, name), body(body) {}
public:
    static const IRType ClassType = IRType::CONSTRUCT;

    virtual ~Construct() {
        for (auto i: body)
            delete i;
    }

    void push_back(IR *i) {
        body.push_back(i);
    }

    IR *back() {
        return this->body.back();
    }

    size_t size() {
        return this->body.size();
    }

    bool empty() {
        return this->body.empty();
    }

    std::list<IR *> get_body() { return this->body; }
};

/**
 * @brief Statements serve for control.
 * Statements cannot appear in expression and example can be break,
 * import, return or assert
 */
class Statement : public IR {
protected:
    Statement(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::STATEMENT;

    virtual void add_annot(Annotation *ann) override {
        (void)ann;
        assert(false && "Adding annotation to a statement");
    }
};

/**
 * @brief Expression represents some value
 */
class Expression : public IR {
protected:
    Expression(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::EXPRESSION;

    virtual void add_annot(Annotation *ann) override {
        (void)ann;
        assert(false && "Adding annotation to an expression");
    }
};

class Module : public Construct {
private:
    SourceFile &src_file;
    bool is_main;
public:
    static const IRType ClassType = IRType::MODULE;

    Module(ustring name, SourceFile &src_file, bool is_main) 
        : Construct(ClassType, name), src_file(src_file), is_main(is_main) {}
    Module(ustring name, SourceFile &src_file, std::list<IR *> body, bool is_main) 
        : Construct(ClassType, name, body), src_file(src_file), is_main(is_main) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<Module>" << name << "{\n";
        for (auto d: body) {
            if (!d) os << "nullptr";
            else os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Space : public Construct {
public:
    static const IRType ClassType = IRType::SPACE;

    Space(ustring name, std::list<IR *> spbody) : Construct(ClassType, name) {
        this->body = spbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "space " << name << " {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Class : public Construct {
private:
    std::vector<Expression *> parents;
public:
    static const IRType ClassType = IRType::CLASS;

    Class(ustring name, std::vector<Expression *> parents, std::list<IR *> clbody) 
        : Construct(ClassType, name), parents(parents) {
        this->body = clbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "class " << name;
        bool first = true;
        for (auto p: parents) {
            if (first) {
                os << " : " << *p;
                first = false;
            }
            else {
                os << ", " << *p;
            }
        }
        os << " {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Argument : public Expression {
private:
    std::vector<Expression *> types;
    Expression *default_value;
    bool vararg;

public:
    static const IRType ClassType = IRType::ARGUMENT;

    Argument(ustring name, std::vector<Expression *> types, Expression *default_value=nullptr)
        : Expression(ClassType, name), types(types), default_value(default_value), vararg(false) {}
    Argument(ustring name) : Expression(ClassType, name), vararg(true) {}
    ~Argument() {
        for (auto t: types) {
            delete t;
        }
        if (default_value)
            delete default_value;
    }

    std::vector<Expression *> get_types() { return this->types; }
    Expression *get_type(unsigned index) { 
        assert(index < types.size() && "Out of bounds access");
        return types[index];
    }
    size_t types_size() { return types.size(); }
    bool is_typed() { return !types.empty(); }
    Expression *get_default_value() { return this->default_value; }
    bool has_default_value() { return this->default_value != nullptr; }
    bool is_vararg() { return this->vararg; }

    virtual inline std::ostream& debug(std::ostream& os) const {
        if (vararg) {
            os << "... " << name;
        }
        else {
            os << name;
            bool first = true;
            for (auto t: types) {
                if (first) {
                    os << ":[" << *t;
                    first = false;
                }
                else
                    os << ", " << *t;
            }
            if (!first)
                os << "]";
            if (default_value)
                os << "=" << *default_value;
        }
        return os;
    }
};

/* TODO: return a vector of names based on the typed args
ustring encode_fun(ustring name, std::vector<Argument *> args) {
    std::stringstream encoded;
    encoded << name << "("

    bool first = true;
    for (auto a : args) {
        assert(a->types_size() <= 1 && "Function has to be specialized to 1 type");
        if (!first) {
            encoded << ",";
        }
        if (a->is_typed()) {
            encoded << a->get_type(0)->get_name();
        }
        else {
            encoded << "_";
        } 
        first = false;
    }

    encoded << ")";
    return encoded.str();
}*/

class Function : public Construct {
private:
    std::vector<Argument *> args;
    bool constructor;

public:
    static const IRType ClassType = IRType::FUNCTION;

    Function(ustring name, std::vector<Argument *> args, std::list<IR *> fnbody, bool constructor=false) 
        : Construct(ClassType, name), args(args), constructor(constructor) {
        this->body = fnbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << (constructor ? "new " : "fun ") << name << "(";
        bool first = true;
        for (auto a: args) {
            if (first) {
                os << *a;
                first = false;
            }
            else {
                os << ", " << *a;
            }
        }
        os << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Else : public Construct {
public:
    static const IRType ClassType = IRType::ELSE;

    Else(std::list<IR *> elbody) : Construct(ClassType, "else") {
        this->body = elbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "else {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class If : public Construct {
private:
    Expression *cond;
    Else *elseBranch;

public:
    static const IRType ClassType = IRType::IF;

    If(Expression *cond, std::list<IR *> ifbody, Else *elseBranch=nullptr) 
           : Construct(ClassType, "if"), cond(cond), elseBranch(elseBranch) {
        this->body = ifbody;
    }
    ~If() {
        delete cond;
        if (elseBranch)
            delete elseBranch;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "if (" << *cond << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        if (elseBranch) {
            os << *elseBranch;
        }
        return os;
    }
};

class Case : public Construct {
private:
    std::vector<Expression *> values;
    bool default_case;

public:
    static const IRType ClassType = IRType::CASE;

    Case(std::vector<Expression *> values, std::list<IR *> csbody, bool default_case=false) 
           : Construct(ClassType, "case"), values(values), default_case(default_case) {
        this->body = csbody;
    }
    ~Case() {
        for (auto v: values) {
            delete v;
        }
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        if (!default_case) {
            os << "case ";
            bool first = true;
            for (auto v: values) {
                if (first) {
                    os << *v;
                    first = false;
                }
                else
                    os << ", " << *v;
            }
        }
        else {
            os << "default";
        }
        os << ": {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Switch : public Construct {
private:
    Expression *val;

public:
    static const IRType ClassType = IRType::SWITCH;

    Switch(Expression *val, std::list<IR *> cases) 
           : Construct(ClassType, "switch"), val(val) {
        this->body = cases;
    }
    ~Switch() {
        delete val;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "switch (" << *val << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Catch : public Construct {
private:
    Argument *arg;

public:
    static const IRType ClassType = IRType::CATCH;

    Catch(Argument *arg, std::list<IR *> ctbody) : Construct(ClassType, "catch"), arg(arg) {
        this->body = ctbody;
    }
    ~Catch() {
        delete arg;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "catch (" << *arg << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Finally : public Construct {
public:
    static const IRType ClassType = IRType::FINALLY;

    Finally(std::list<IR *> fnbody) : Construct(ClassType, "finally") {
        this->body = fnbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "finally {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Try : public Construct {
private:
    std::vector<Catch *> catches;
    Finally *finallyStmt;

public:
    static const IRType ClassType = IRType::TRY;

    Try(std::vector<Catch *> catches, std::list<IR *> trbody, Finally *finallyStmt=nullptr) 
           : Construct(ClassType, "try"), catches(catches), finallyStmt(finallyStmt) {
        this->body = trbody;
    }
    ~Try() {
        for (auto c : catches) {
            delete c;
        }
        if (finallyStmt)
            delete finallyStmt;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "try {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        for (auto c : catches) {
            os << *c;
        }
        if (finallyStmt)
            os << *finallyStmt;
        return os;
    }
};

class While : public Construct {
private:
    Expression *cond;

public:
    static const IRType ClassType = IRType::WHILE;

    While(Expression *cond, std::list<IR *> whbody) 
           : Construct(ClassType, "while"), cond(cond) {
        this->body = whbody;
    }
    ~While() {
        delete cond;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "while (" << *cond << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class DoWhile : public Construct {
private:
    Expression *cond;

public:
    static const IRType ClassType = IRType::DO_WHILE;

    DoWhile(Expression *cond, std::list<IR *> whbody) 
           : Construct(ClassType, "do-while"), cond(cond) {
        this->body = whbody;
    }
    ~DoWhile() {
        delete cond;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "do {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "while (" << *cond << ")";
        return os;
    }
};

class ForLoop : public Construct {
private:
    Expression *iterator;
    Expression *collection;

public:
    static const IRType ClassType = IRType::FOR_LOOP;

    ForLoop(Expression *iterator, Expression *collection, std::list<IR *> frbody) 
           : Construct(ClassType, "for"), iterator(iterator), collection(collection) {
        this->body = frbody;
    }
    ~ForLoop() {
        delete iterator;
        delete collection;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "for (" << *iterator << ": " << *collection << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        return os;
    }
};

class Enum : public IR {
private:
    std::vector<ustring> values;
public:
    static const IRType ClassType = IRType::ENUM;

    Enum(ustring name, std::vector<ustring> values) : IR(ClassType, name), values(values) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "enum " << name << " {\n";
        for (auto v: values) {
            os << v << "\n";
        }
        os << "}";
        return os;
    }
};

class Import : public Statement {
private:
    std::vector<Expression *> names;
    std::vector<ustring> aliases;

public:
    static const IRType ClassType = IRType::IMPORT;

    Import(std::vector<Expression *> names, std::vector<ustring> aliases) 
           : Statement(ClassType, "import"), names(names), aliases(aliases) {}
    ~Import() {
        for (auto name : names)
            delete name;
    }

    std::vector<Expression *> get_names() { return names; }
    std::vector<ustring> get_aliases() { return aliases; }

    Expression *get_name(unsigned index) {
        assert(index < names.size() && "Out of bounds index");
        return names[index];
    }

    ustring get_alias(unsigned index) {
        assert(index < aliases.size() && "Out of bounds index");
        return aliases[index];
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "import ";
        bool first = true;
        for (unsigned i = 0; i < names.size(); ++i) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << *names[i];
            if (!aliases[i].empty())
                os << " as " << aliases[i];
        }
        return os;
    }
};

class Assert : public Statement {
private:
    Expression *cond;
    Expression *msg;

public:
    static const IRType ClassType = IRType::ASSERT;

    Assert(Expression *cond, Expression *msg=nullptr) 
           : Statement(ClassType, "assert"), cond(cond), msg(msg) {}
    ~Assert() {
        delete cond;
        if (msg)
            delete msg;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "assert(" << *cond;
        if (msg)
            os << ", " << *msg << ")";
        else
            os << ")";
        return os;
    }
};

class Raise : public Statement {
private:
    Expression *exception;

public:
    static const IRType ClassType = IRType::RAISE;

    Raise(Expression *exception) : Statement(ClassType, "raise"), exception(exception) {}
    ~Raise() {
        delete exception;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "raise " << *exception;
        return os;
    }

    Expression *get_exception() { return this->exception; }
};

class Return : public Statement {
private:
    Expression *expr;

public:
    static const IRType ClassType = IRType::RETURN;

    Return(Expression *expr) : Statement(ClassType, "return"), expr(expr) {}
    ~Return() {
        delete expr;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "return " << *expr;
        return os;
    }

    Expression *get_expr() { return this->expr; }
};

class Break : public Statement {
public:
    static const IRType ClassType = IRType::BREAK;

    Break() : Statement(ClassType, "break") {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "break";
        return os;
    }
};

class Continue : public Statement {
public:
    static const IRType ClassType = IRType::CONTINUE;

    Continue() : Statement(ClassType, "break") {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "continue";
        return os;
    }
};

class Annotation : public Statement {
private:
    Expression *value;
    bool inner;
public:
    static const IRType ClassType = IRType::ANNOTATION;

    Annotation(Expression *value, bool inner) 
        : Statement(ClassType, "annotation"), value(value), inner(inner) {}
    ~Annotation() {
        delete value;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << (inner ? "@!" : "@") << *value;
        return os;
    }
};

class EndOfFile : public Statement {
public:
    static const IRType ClassType = IRType::END_OF_FILE;

    EndOfFile() : Statement(ClassType, "<end-of-file>") {}
};

/**
 * Types of operators
 */
enum OperatorKind {
    OP_UNKNOWN,
    OP_CONCAT, ///< ++
    OP_EXP,    ///< ^
    OP_PLUS,   ///< `+`
    OP_MINUS,  ///< `-`, Unary and Binary
    OP_DIV,    ///< /
    OP_MUL,    ///< `*`
    OP_MOD,    ///< %
    OP_SET,         ///< =
    OP_SET_CONCAT,  ///< ++=
    OP_SET_EXP,     ///< ^= 
    OP_SET_PLUS,    ///< +=
    OP_SET_MINUS,   ///< -=
    OP_SET_DIV,     ///< /=
    OP_SET_MUL,     ///< *=
    OP_SET_MOD,     ///< %=
    OP_SILENT, ///< ~, Unary
    OP_EQ,     ///< ==
    OP_NEQ,    ///< !=
    OP_BT,     ///< `>`
    OP_LT,     ///< <
    OP_BEQ,    ///< >=
    OP_LEQ,    ///< <=
    OP_SHORT_C_AND, ///< &&
    OP_SHORT_C_OR,  ///< ||
    OP_AND,    ///< and
    OP_OR,     ///< or
    OP_NOT,    ///< not, Unary
    OP_XOR,    ///< xor
    OP_IN,     ///< in
    OP_ACCESS, ///< `.`
    OP_SUBSC,  ///< [] or [..]
    OP_SCOPE,  ///< :: 
    OP_UNPACK  ///< <<
};

/**
 * Operator in an expression
 */
class Operator {
private:
    OperatorKind kind;
public:
    Operator(OperatorKind kind) : kind(kind) {}

    OperatorKind get_kind() const { return kind; }

    inline std::ostream& debug(std::ostream& os) const { 
        switch(kind) {
        case OP_CONCAT: os << "++"; break;
        case OP_EXP: os << "^"; break;
        case OP_PLUS: os << "+"; break;
        case OP_MINUS: os << "-"; break;
        case OP_DIV: os << "/"; break;
        case OP_MUL: os << "*"; break;
        case OP_MOD: os << "%"; break;
        case OP_SET: os << "="; break;
        case OP_SET_CONCAT: os << "++="; break;
        case OP_SET_EXP: os << "^="; break;
        case OP_SET_PLUS: os << "+="; break;
        case OP_SET_MINUS: os << "-="; break;
        case OP_SET_DIV: os << "/="; break;
        case OP_SET_MUL: os << "+="; break;
        case OP_SET_MOD: os << "%="; break;
        case OP_SILENT: os << "~"; break;
        case OP_EQ: os << "=="; break;
        case OP_NEQ: os << "!="; break;
        case OP_BT: os << ">"; break;
        case OP_LT: os << "<"; break;
        case OP_BEQ: os << ">="; break;
        case OP_LEQ: os << "<="; break;
        case OP_SHORT_C_AND: os << "&&"; break;
        case OP_SHORT_C_OR: os << "||"; break;
        case OP_AND: os << "and"; break;
        case OP_OR: os << "or"; break;
        case OP_NOT: os << "not"; break;
        case OP_XOR: os << "xor"; break;
        case OP_IN: os << "in"; break;
        case OP_ACCESS: os << "."; break;
        case OP_SUBSC: os << "[]"; break;
        case OP_SCOPE: os << "::"; break;
        case OP_UNPACK: os << "<<"; break;
        case OP_UNKNOWN: os << "unknown"; break;
        default:
            assert(false && "Missing operator in debug for Operator class"); 
            os << "unknown operator"; break;
        }
        return os;
    }
};

inline std::ostream& operator<< (std::ostream& os, const Operator &op) {
    return op.debug(os);
}

class BinaryExpr : public Expression {
private:
    Expression *left;
    Expression *right;
    const Operator op;
public:
    static const IRType ClassType = IRType::BINARY_EXPR;

    BinaryExpr(Expression *left, Expression *right, Operator op) 
              : Expression(ClassType, "<binary-expression>"),
                left(left), right(right), op(op) {}
    ~BinaryExpr() {
        delete left;
        delete right;
    }

    Expression *get_left() { return this->left; }
    Expression *get_right() { return this->right; }
    Operator get_op() { return this->op; }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "(" << *left << " " << op << " " << *right << ")";
        return os;
    }
};

class UnaryExpr : public Expression {
private:
    Expression *expr;
    const Operator op;
public:
    static const IRType ClassType = IRType::UNARY_EXPR;

    UnaryExpr(Expression *expr, Operator op) 
             : Expression(ClassType, "<unary-expression>"), 
               expr(expr), op(op) {}
    ~UnaryExpr() {
        delete expr;
    }

    Expression *get_expr() { return this->expr; }
    Operator get_op() { return this->op; }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "(" << op << " " << *expr << ")";
        return os;
    }
};

class Variable : public Expression {
public:
    static const IRType ClassType = IRType::VARIABLE;

    Variable(ustring name) : Expression(ClassType, name) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << name;
        return os;
    }
};

class TernaryIf : public Expression {
private:
    Expression *condition;
    Expression *value_true;
    Expression *value_false;
public:
    static const IRType ClassType = IRType::TERNARY_IF;

    TernaryIf(Expression *condition, Expression *value_true, Expression *value_false)
        : Expression(ClassType, "<ternary-if>"),
          condition(condition),
          value_true(value_true),
          value_false(value_false) {}
    ~TernaryIf() {
        delete condition;
        delete value_true;
        delete value_false;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "(" << *condition << " ? " << *value_true << " : " << *value_false << ")";
        return os;
    }
};

class Range : public Expression {
private:
    Expression *start;
    Expression *end;
    Expression *second; // Might be nullptr
public:
    static const IRType ClassType = IRType::RANGE;

    Range(Expression *start, Expression *end, Expression *second=nullptr)
        : Expression(ClassType, "<range>"),
          start(start),
          end(end),
          second(second) {}
    ~Range() {
        delete start;
        delete end;
        if (second)
            delete second;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        if (second)
            os << "(" << *start << ", " << *second << ".." << *end << ")";
        else
            os << "(" << *start << ".." << *end << ")";
        return os;
    }
};

class Call : public Expression {
private:
    Expression *fun;
    std::vector<Expression *> args;
public:
    static const IRType ClassType = IRType::CALL;

    Call(Expression *fun, std::vector<Expression *> args)
        : Expression(ClassType, "<call>"), fun(fun), args(args) {}
    ~Call() {
        delete fun;
        for (auto a : args) {
            delete a;
        }
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << *fun << "(";
        bool first = true;
        for (auto a: args) {
            if (first) {
                os << *a;
                first = false;
                continue;
            }
            os << ", " << *a; 
        }
        os << ")"; 
        return os;
    }
};

class IntLiteral : public Expression {
private:
    opcode::IntConst value;
public:
    static const IRType ClassType = IRType::INT_LITERAL;

    IntLiteral(opcode::IntConst value) : Expression(ClassType, "<int-literal>"), value(value) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << value;
        return os;
    }

    opcode::IntConst get_value() { return this->value; }
};

class FloatLiteral : public Expression {
private:
    opcode::FloatConst value;
public:
    static const IRType ClassType = IRType::FLOAT_LITERAL;

    FloatLiteral(opcode::FloatConst value) : Expression(ClassType, "<float-literal>"), value(value) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << value;
        return os;
    }

    opcode::FloatConst get_value() { return this->value; }
};

class BoolLiteral : public Expression {
private:
    opcode::BoolConst value;
public:
    static const IRType ClassType = IRType::BOOL_LITERAL;

    BoolLiteral(opcode::BoolConst value) : Expression(ClassType, "<bool-literal>"), value(value) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << (value ? "true" : "false");
        return os;
    }

    opcode::BoolConst get_value() { return this->value; }
}; 

class StringLiteral : public Expression {
private:
    opcode::StringConst value;
public:
    static const IRType ClassType = IRType::STRING_LITERAL;

    StringLiteral(opcode::StringConst value) : Expression(ClassType, "<string-literal>"), value(value) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "\"" << utils::sanitize(value) << "\"";
        return os;
    }

    opcode::StringConst get_value() { return this->value; }
};


class Note : public Expression {
private:
    Expression *prefix;
    StringLiteral *note;
public:
    static const IRType ClassType = IRType::NOTE;

    Note(Expression *prefix, StringLiteral *note) : Expression(ClassType, "<note>"), prefix(prefix), note(note) {}
    ~Note() {
        delete prefix;
        delete note;
    }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << *prefix << *note;
        return os;
    }
};

class NilLiteral : public Expression {
public:
    static const IRType ClassType = IRType::NIL_LITERAL;

    NilLiteral() : Expression(ClassType, "<nil-literal>") {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "nil";
        return os;
    }
}; 

}

// Helper functions
template<class T>
bool isa(ir::IR& i) {
    return i.get_type() == T::ClassType;
}
template<> inline bool isa<ir::Expression>(ir::IR& i) {
    return i.get_type() >= ir::IRType::BINARY_EXPR && i.get_type() <= ir::IRType::NIL_LITERAL;
}

template<class T>
bool isa(ir::IR* i) {
    return i->get_type() == T::ClassType;
}
template<> inline bool isa<ir::Expression>(ir::IR* i) {
    return i->get_type() >= ir::IRType::BINARY_EXPR && i->get_type() <= ir::IRType::NIL_LITERAL;
}

template<class T>
T *dyn_cast(ir::IR* i) {
    if (!isa<T>(i)) return nullptr;
    return dynamic_cast<T *>(i);
}
template<> inline ir::Expression *dyn_cast<>(ir::IR* i) {
    if (isa<ir::Expression>(i)) {
        if (auto e = dyn_cast<ir::BinaryExpr>(i)) return e;
        else if (auto e = dyn_cast<ir::UnaryExpr>(i)) return e;
        else if (auto e = dyn_cast<ir::Variable>(i)) return e;
        else if (auto e = dyn_cast<ir::Argument>(i)) return e;
        else if (auto e = dyn_cast<ir::Note>(i)) return e;
        else if (auto e = dyn_cast<ir::TernaryIf>(i)) return e;
        else if (auto e = dyn_cast<ir::Range>(i)) return e;
        else if (auto e = dyn_cast<ir::Call>(i)) return e;
        else if (auto e = dyn_cast<ir::IntLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::FloatLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::BoolLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::StringLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::NilLiteral>(i)) return e;
        assert(false && "TODO: Unimplemented dyn_cast to Expression");
    }
    return nullptr;
}
template<> inline ir::Construct *dyn_cast<>(ir::IR* i) {
    (void)i;
    assert(false && "TODO: Unimplemented dyn_cast to Construct");
    return nullptr;
}
template<> inline ir::Statement *dyn_cast<>(ir::IR* i) {
    (void)i;
    assert(false && "TODO: Unimplemented dyn_cast to Statement");
    return nullptr;
}

}

#endif//_IR_HPP_