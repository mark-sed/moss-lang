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

namespace moss {

namespace ir {

enum class IRType {
    CONSTRUCT,
    EXPRESSION,
    STATEMENT,

    MODULE,
    SPACE,

    ASSERT,
    RAISE,
    RETURN,
    BREAK,
    CONTINUE,
    ANNOTATION,

    BINARY_EXPR,
    UNARY_EXPR,

    VARIABLE,
    TERNARY_IF,
    RANGE,
    CALL,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    NIL_LITERAL,

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
        os << "}\n";
        return os;
    }
};

class Space : public Construct {
public:
    static const IRType ClassType = IRType::SPACE;

    Space(ustring name) : Construct(ClassType, name) {}

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "space " << name;
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
    OP_NEG,    ///< `-`
    OP_CONCAT, ///< ++
    OP_EXP,    ///< ^
    OP_PLUS,   ///< `+`
    OP_MINUS,  ///< `-`
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
    OP_SILENT, ///< ~
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
    OP_NOT,    ///< not
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
        case OP_NEG: os << "-"; break;
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

template<class T>
bool isa(ir::IR* i) {
    return i->get_type() == T::ClassType;
}

template<class T>
T *dyn_cast(ir::IR* i) {
    if (!isa<T>(i)) return nullptr;
    return dynamic_cast<T *>(i);
}

}

#endif//_IR_HPP_