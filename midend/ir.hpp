/// 
/// \file ir.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Internal representation of moss constructs.
/// This IR is then lowered into bytecode.
/// 

#ifndef _IR_HPP_
#define _IR_HPP_

#include "source.hpp"
#include "commons.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <cassert>
#include <list>
#include <vector>

#include <sstream>
#include "logging.hpp"

namespace moss {

/// Internal representation of code namespace
namespace ir {

/// IR class type for easy casting and checking
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
    DOC_STRING,

    BINARY_EXPR, // Start of Expresion IDs -- Add any new to dyn_cast bellow!
    UNARY_EXPR,
    VARIABLE,
    MULTI_VAR,
    ALL_SYMBOLS,
    ARGUMENT,
    LAMBDA,
    NOTE,
    LIST,
    DICT,
    TERNARY_IF,
    RANGE,
    CALL,
    THIS_LITERAL,
    SUPER_LITERAL,
    OPERATOR_LITERAL,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    NIL_LITERAL, // End of Expression IDs

    END_OF_FILE,
};

class Annotation;

/// Base class for any IR value
class IR {
protected:
    IRType ir_type;
    ustring name;
    std::list<Annotation *> annotations;
    ustring documentation;

    IR(IRType ir_type, ustring name) : ir_type(ir_type), name(name), documentation() {}
public:
    virtual ~IR() {}

    IRType get_type() { return ir_type; }
    ustring get_name() { return name; }
    virtual ustring as_string() {
        return name;
    }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<IR: " << name << ">";
        return os;
    }

    virtual void add_annotation(Annotation *ann) {
        annotations.push_back(ann);
    }

    virtual void set_annotations(std::list<Annotation *> a) {
        annotations = a;
    }

    virtual bool can_be_annotated() { return false; }
    virtual bool can_be_documented() { return false; }

    void add_documentation(ustring d) {
        this->documentation += d;
    }

    ustring get_documentation() { return this->documentation; }

    std::list<Annotation *> get_annotations() { return this->annotations; }
};

inline std::ostream& operator<< (std::ostream& os, IR &ir) {
    return ir.debug(os);
}
 
/// \brief IRs that are constructs
/// Constructs cannot appear in an expression and have certain control flow
/// and body.
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
    void set_body(std::list<IR *> b) { this->body = b; }
};

/// \brief Statements serve for control.
/// Statements cannot appear in expression and example can be break,
/// import, return or assert
class Statement : public IR {
protected:
    Statement(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::STATEMENT;

    virtual void add_annotation(Annotation *ann) override {
        (void)ann;
        assert(false && "Adding annotation to a statement");
    }
};

/// \brief Expression represents some value 
class Expression : public IR {
protected:
    Expression(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::EXPRESSION;

    virtual void add_annotation(Annotation *ann) override {
        (void)ann;
        assert(false && "Adding annotation to an expression");
    }
};

class Module : public Construct {
public:
    static const IRType ClassType = IRType::MODULE;

    Module(ustring name) : Construct(ClassType, name) {}
    Module(ustring name, std::list<IR *> body) : Construct(ClassType, name, body) {}

    virtual bool can_be_annotated() override { return true; }
    virtual bool can_be_documented() override { return true; }

    virtual std::ostream& debug(std::ostream& os) const override;
};

class Space : public Construct {
private:
    static unsigned long annonymous_id;
    bool anonymous;
public:
    static const IRType ClassType = IRType::SPACE;

    Space(ustring name) : Construct(ClassType, name) {
        if (name.empty()) {
            this->anonymous = true;
            this->name = std::to_string(annonymous_id++) + "s";
        }
    }

    bool is_anonymous() {
        return this->anonymous;
    }

    virtual bool can_be_annotated() override { return true; }
    virtual bool can_be_documented() override { return true; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Class(ustring name, std::vector<Expression *> parents) 
        : Construct(ClassType, name), parents(parents) {
    }
    ~Class() {
        for (auto p : parents)
            delete p;
    }

    std::vector<Expression *> get_parents() { return this->parents; }
    virtual bool can_be_annotated() override { return true; }
    virtual bool can_be_documented() override { return true; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    Argument(ustring name) : Expression(ClassType, name), default_value(nullptr), vararg(true) {}
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    Function(ustring name, std::vector<Argument *> args, bool constructor=false) 
        : Construct(ClassType, name), args(args), constructor(constructor) {
    }
    ~Function() {
        for (auto a : args)
            delete a;
    }

    std::vector<Argument *> get_args() { return this->args; }
    void set_constructor(bool c) { this->constructor = c; }
    bool is_constructor() { return this->constructor; }

    virtual bool can_be_annotated() override { return true; }
    virtual bool can_be_documented() override { return true; }

    virtual std::ostream& debug(std::ostream& os) const override;
};

inline ustring encode_fun_args(std::vector<Argument *> args) {
    ustring txt = "";
    bool first = true;
    for (auto a: args) {
        if (first) {
            txt += a->get_name();
            first = false;
        }
        else {
            txt += ","+a->get_name();
        }
    }
    return txt;
}

class Else : public Construct {
public:
    static const IRType ClassType = IRType::ELSE;

    Else(std::list<IR *> elbody) : Construct(ClassType, "else") {
        this->body = elbody;
    }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    Else *else_branch;

public:
    static const IRType ClassType = IRType::IF;

    If(Expression *cond, std::list<IR *> ifbody, Else *else_branch=nullptr) 
           : Construct(ClassType, "if"), cond(cond), else_branch(else_branch) {
        this->body = ifbody;
    }
    ~If() {
        delete cond;
        if (else_branch)
            delete else_branch;
    }

    Expression *get_cond() { return this->cond; }
    Else *get_else() { return this->else_branch; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "if (" << *cond << ") {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        if (else_branch) {
            os << *else_branch;
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

    std::vector<Expression *> get_values() { return values; }
    bool is_default_case() { return default_case; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    Expression *cond;

public:
    static const IRType ClassType = IRType::SWITCH;

    Switch(Expression *cond, std::list<IR *> cases) 
           : Construct(ClassType, "switch"), cond(cond) {
        this->body = cases;
    }
    ~Switch() {
        delete cond;
    }

    Expression *get_cond() { return this->cond; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "switch (" << *cond << ") {\n";
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

    Argument *get_arg() { return arg; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    Finally *finally_stmt;

public:
    static const IRType ClassType = IRType::TRY;

    Try(std::vector<Catch *> catches, std::list<IR *> trbody, Finally *finally_stmt=nullptr) 
           : Construct(ClassType, "try"), catches(catches), finally_stmt(finally_stmt) {
        this->body = trbody;
    }
    ~Try() {
        for (auto c : catches) {
            delete c;
        }
        if (finally_stmt)
            delete finally_stmt;
    }

    std::vector<Catch *>& get_catches() { return catches; }
    Finally *get_finally() { return finally_stmt; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "try {\n";
        for (auto d: body) {
            os << *d << "\n";
        }
        os << "}";
        for (auto c : catches) {
            os << *c;
        }
        if (finally_stmt)
            os << *finally_stmt;
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

    Expression *get_cond() { return this->cond; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Expression *get_cond() { return this->cond; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Expression *get_iterator() { return this->iterator; }
    Expression *get_collection() { return this->collection; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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
    // TODO: This could be List to simplify codegen as it needs to be converted anyway
    std::vector<ustring> values;
public:
    static const IRType ClassType = IRType::ENUM;

    Enum(ustring name, std::vector<ustring> values) : IR(ClassType, name), values(values) {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "enum " << name << " {\n";
        for (auto v: values) {
            os << v << "\n";
        }
        os << "}";
        return os;
    }

    std::vector<ustring> get_values() {
        return this->values;
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Expression *get_cond() { return this->cond; }
    Expression *get_msg() { return this->msg; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "assert(" << *cond;
        if (msg)
            os << ", " << *msg << ")";
        else
            os << ")";
        return os;
    }
};

/*class DocString : public Statement {
private:
    ustring value;

public:
    static const IRType ClassType = IRType::DOC_STRING;

    DocString(ustring value) : Statement(ClassType, "<doc-string>"), value(value) {}
    ~DocString() {
    }

    ustring get_value() { return this->value; }

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "d\"" << value << "\"";
        return os;
    }
};*/

class Raise : public Statement {
private:
    Expression *exception;

public:
    static const IRType ClassType = IRType::RAISE;

    Raise(Expression *exception) : Statement(ClassType, "raise"), exception(exception) {}
    ~Raise() {
        delete exception;
    }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "return " << *expr;
        return os;
    }

    Expression *get_expr() { return this->expr; }
};

class Break : public Statement {
public:
    static const IRType ClassType = IRType::BREAK;

    Break() : Statement(ClassType, "break") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "break";
        return os;
    }
};

class Continue : public Statement {
public:
    static const IRType ClassType = IRType::CONTINUE;

    Continue() : Statement(ClassType, "break") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Annotation(ustring name, Expression *value, bool inner) 
        : Statement(ClassType, name), value(value), inner(inner) {}
    ~Annotation() {
        delete value;
    }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << (inner ? "@!" : "@") << name;
        if (value)
            os << "(" << *value << ")";
        return os;
    }

    Expression *get_value() { return this->value; }
    bool is_inner() { return this->inner; }
};

class EndOfFile : public Statement {
public:
    static const IRType ClassType = IRType::END_OF_FILE;

    EndOfFile() : Statement(ClassType, "<end-of-file>") {}
};

/// Types of operators
/// \note Beware that the order is used for checks
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

/// Operator in an expression
class Operator {
private:
    OperatorKind kind;
public:
    Operator(OperatorKind kind) : kind(kind) {}

    OperatorKind get_kind() const { return kind; }

    ustring as_string() const {
        switch(kind) {
            case OP_CONCAT: return "++"; 
            case OP_EXP: return "^"; 
            case OP_PLUS: return "+"; 
            case OP_MINUS: return "-"; 
            case OP_DIV: return "/"; 
            case OP_MUL: return "*"; 
            case OP_MOD: return "%"; 
            case OP_SET: return "="; 
            case OP_SET_CONCAT: return "++="; 
            case OP_SET_EXP: return "^="; 
            case OP_SET_PLUS: return "+="; 
            case OP_SET_MINUS: return "-="; 
            case OP_SET_DIV: return "/="; 
            case OP_SET_MUL: return "+="; 
            case OP_SET_MOD: return "%="; 
            case OP_SILENT: return "~"; 
            case OP_EQ: return "=="; 
            case OP_NEQ: return "!="; 
            case OP_BT: return ">"; 
            case OP_LT: return "<"; 
            case OP_BEQ: return ">="; 
            case OP_LEQ: return "<="; 
            case OP_SHORT_C_AND: return "&&"; 
            case OP_SHORT_C_OR: return "||"; 
            case OP_AND: return "and"; 
            case OP_OR: return "or"; 
            case OP_NOT: return "not"; 
            case OP_XOR: return "xor"; 
            case OP_IN: return "in"; 
            case OP_ACCESS: return "."; 
            case OP_SUBSC: return "[]"; 
            case OP_SCOPE: return "::"; 
            case OP_UNPACK: return "<<"; 
            case OP_UNKNOWN: return "unknown"; 
            default:
                assert(false && "Missing operator in debug for Operator class"); 
                return "unknown operator"; 
        }
    }

    inline std::ostream& debug(std::ostream& os) const { 
        os << as_string();
        return os;
    }
};

inline bool is_set_op(Operator op) {
    return op.get_kind() >= OperatorKind::OP_SET && op.get_kind() <= OperatorKind::OP_SET_MOD;
}

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
        // We need to check since in parse we might generate binexpr to then
        // extract the left and right and reassign it, in such case the old
        // binexpr is deleted and left and right is nullptr
        if (left)
            delete left;
        if (right) {
            delete right;
        }
    }

    Expression *get_left() { return this->left; }
    Expression *get_right() { return this->right; }
    void set_left(Expression *e) { this->left = e; }
    void set_right(Expression *e) { this->right = e; }
    Operator get_op() { return this->op; }

    virtual ustring as_string() override {
        return left->as_string() + op.as_string() + right->as_string();
    }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual ustring as_string() override {
        return op.as_string() + expr->as_string();
    }

    Expression *get_expr() { return this->expr; }
    Operator get_op() { return this->op; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "(" << op << " " << *expr << ")";
        return os;
    }
};

class Variable : public Expression {
private:
    bool non_local;
public:
    static const IRType ClassType = IRType::VARIABLE;

    Variable(ustring name, bool non_local=false) : Expression(ClassType, name), non_local(non_local) {}

    bool is_non_local() { return this->non_local; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << (non_local ? "$" : "") << name;
        return os;
    }
};

class Multivar : public Expression {
private:
    std::vector<ir::Expression *> vars;
public:
    static const IRType ClassType = IRType::MULTI_VAR;

    Multivar(std::vector<ir::Expression *> vars) : Expression(ClassType, "<multivar>"), vars(vars) {}
    ~Multivar() {
        for (auto v: vars)
            delete v;
    }

    std::vector<ir::Expression *> get_vars() { return this->vars; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        bool first = true;
        for (auto v: vars) {
            if (first) {
                os << "(" << *v;
                first = false;
            } else {
                os << "," << *v;
            }
        }
        os << ")";
        return os;
    }
};

/// IR for denoting * in import of all symbols
class AllSymbols : public Expression {
public:
    static const IRType ClassType = IRType::ALL_SYMBOLS;

    AllSymbols() : Expression(ClassType, "*") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "*";
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

    Expression *get_condition() { return condition; }
    Expression *get_value_true() { return value_true; }
    Expression *get_value_false() { return value_false; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "(" << *condition << " ? " << *value_true << " : " << *value_false << ")";
        return os;
    }
};

class Lambda : public Expression {
private:
    static unsigned long annonymous_id;
    std::vector<Argument *> args;
    Expression *body;
public:
    static const IRType ClassType = IRType::LAMBDA;

    Lambda(ustring name, std::vector<Argument *> args, Expression *body) 
        : Expression(ClassType, name), args(args), body(body) {
        if (name.empty()) {
            this->name = std::to_string(annonymous_id++) + "l";
        }
    }
    ~Lambda() {
        for (auto a : this->args)
            delete a;
        delete body;
    }

    virtual void add_annotation(Annotation *ann) override {
        annotations.push_back(ann);
    }
    virtual bool can_be_annotated() override { return true; }
    virtual bool can_be_documented() override { return false; }

    std::vector<Argument *> get_args() { return this->args; }
    Expression *get_body() { return this->body; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "(fun " << name << "(";
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
        os << ") = " << *body << ")";
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

    Expression *get_start() { return start; }
    Expression *get_end() { return end; }
    Expression *get_second() { return second; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    Expression *get_fun() { return this->fun; }
    std::vector<Expression *> get_args() { return this->args; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
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

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "\"" << utils::sanitize(value) << "\"";
        return os;
    }

    opcode::StringConst get_value() { return this->value; }
};

class NilLiteral : public Expression {
public:
    static const IRType ClassType = IRType::NIL_LITERAL;

    NilLiteral() : Expression(ClassType, "<nil-literal>") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "nil";
        return os;
    }
};

class ThisLiteral : public Expression {
public:
    static const IRType ClassType = IRType::THIS_LITERAL;

    ThisLiteral() : Expression(ClassType, "<this-literal>") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "this";
        return os;
    }
};

class SuperLiteral : public Expression {
public:
    static const IRType ClassType = IRType::SUPER_LITERAL;

    SuperLiteral() : Expression(ClassType, "<super-literal>") {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "super";
        return os;
    }
};

/// Holds operator as a name for function - `(+)`
class OperatorLiteral : public Expression {
private:
    Operator op;
public:
    static const IRType ClassType = IRType::OPERATOR_LITERAL;

    OperatorLiteral(Operator op) : Expression(ClassType, "<operator-literal>"), op(op) {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "(" << op << ")";
        return os;
    }

    Operator get_op() { return this->op; }
};

class Note : public Expression {
private:
    ustring prefix;
    StringLiteral *note;
public:
    static const IRType ClassType = IRType::NOTE;

    Note(ustring prefix, StringLiteral *note) : Expression(ClassType, "<note>"), prefix(prefix), note(note) {}
    ~Note() {
        delete note;
    }

    ustring get_prefix() { return this->prefix; }
    StringLiteral *get_note() { return this->note; }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << prefix << *note;
        return os;
    }
};

class List : public Expression {
private:
    static unsigned long annonymous_id;
    std::vector<Expression *> value;
    bool comprehension;
    Expression *result;
    Expression *else_result;
    Expression *condition;
    std::vector<Expression *> assignments;
    IR *list_compr_var;
    IR *list_compr_for;
    ustring compr_result_name;
public:
    static const IRType ClassType = IRType::LIST;

    List(std::vector<Expression *> value) 
        : Expression(ClassType, "<list>"), value(value), comprehension(false),
          result(nullptr), else_result(nullptr), condition(nullptr),
          list_compr_var(nullptr), list_compr_for(nullptr) {}
    List(Expression *result, std::vector<Expression *> assignments,
         Expression *condition, Expression *else_result)
        : Expression(ClassType, "<list>"), comprehension(true), result(result),
          else_result(else_result), condition(condition), assignments(assignments),
          list_compr_var(nullptr), list_compr_for(nullptr) {
        assert((!else_result || (else_result && condition)) && "else without condition in list comprehension");
        assert(result && "comprehension without result");
        this->compr_result_name = std::to_string(annonymous_id++) + "cl";
    }
    ~List() {
        for (auto v: value)
            delete v;
        if (comprehension) {
            if (list_compr_var) {
                assert(list_compr_for);
                delete list_compr_for;
                delete list_compr_var;
            } else {
                if (result)
                    delete result;
                if (else_result)
                    delete else_result;
                if (condition)
                    delete condition;
                for (auto a: assignments)
                    delete a;
            }
        }
    }

    std::list<IR *> as_for();

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "[";
        if (!comprehension) {
            bool first = true;
            for (auto v: value) {
                if (first) {
                    os << *v;
                    first = false;
                }
                else {
                    os << ", " << *v;
                }
            }
        }
        else {
            os << *result;
            if (condition) {
                os << " if(" << *condition << ")";
                if (else_result) {
                    os << " else " << *else_result;
                }
            }
            os << " : ";
            bool first = true;
            for (auto a: assignments) {
                if (first) {
                    os << *a;
                    first = false;
                }
                else {
                    os << ", " << *a;
                }
            }
        }
        os << "]";
        return os;
    }

    std::vector<Expression *> get_value() { return this->value; }
    bool is_comprehension() { return this->comprehension; }
    Expression *get_result() { return this->result; };
    Expression *get_else_result() { return this->else_result; };
    Expression *get_condition() { return this->condition; };
    std::vector<Expression *> get_assignments() { return this->assignments; };
    ustring get_compr_result_name() {
        assert(comprehension && "extracting list result variable name from non comprehended list");
        return this->compr_result_name;
    }
};

class Dict : public Expression {
private:
    std::vector<Expression *> keys;
    std::vector<Expression *> values;
public:
    static const IRType ClassType = IRType::DICT;

    Dict(std::vector<Expression *> keys, std::vector<Expression *> values)
        : Expression(ClassType, "<dict>"), keys(keys), values(values) {
        assert((keys.size() == values.size()) && "mismatched amount of keys to values");
    }
    ~Dict() {
        for (auto i : keys)
            delete i;
        for (auto i : values)
            delete i;
    }

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << "{";
        if (keys.empty())
            os << ":";
        for (unsigned i = 0; i < keys.size(); ++i) {
            if (i == 0) {
                os << *keys[i] << ": " << *values[i];
            }
            else {
                os << ", " << *keys[i] << ": " << *values[i];
            }
        }
        os << "}";
        return os;
    }

    std::vector<Expression *> get_keys() { return this->keys; }
    std::vector<Expression *> get_values() { return this->values; }
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
        else if (auto e = dyn_cast<ir::AllSymbols>(i)) return e;
        else if (auto e = dyn_cast<ir::Argument>(i)) return e;
        else if (auto e = dyn_cast<ir::Lambda>(i)) return e;
        else if (auto e = dyn_cast<ir::Note>(i)) return e;
        else if (auto e = dyn_cast<ir::List>(i)) return e;
        else if (auto e = dyn_cast<ir::Dict>(i)) return e;
        else if (auto e = dyn_cast<ir::TernaryIf>(i)) return e;
        else if (auto e = dyn_cast<ir::Range>(i)) return e;
        else if (auto e = dyn_cast<ir::Call>(i)) return e;
        else if (auto e = dyn_cast<ir::ThisLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::SuperLiteral>(i)) return e;
        else if (auto e = dyn_cast<ir::OperatorLiteral>(i)) return e;
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

namespace ir {

inline bool can_be_annotated(IR *decl) {
    return isa<Function>(decl) || isa<Module>(decl);
}

}

}

#endif//_IR_HPP_