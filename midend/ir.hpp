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
    END_OF_FILE,

    BINARY_EXPR,
    VARIABLE,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    NIL_LITERAL
};

/** Base class for any IR value */
class IR {
protected:
    IRType ir_type;
    ustring name;

    IR(IRType ir_type, ustring name) : ir_type(ir_type), name(name) {}
public:
    virtual ~IR() {}

    IRType get_type() { return ir_type; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<IR>" << name;
        return os;
    }
};

inline std::ostream& operator<< (std::ostream& os, IR &ir) {
    return ir.debug(os);
}

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
};

/**
 * @brief Expression represents some value
 */
class Expression : public IR {
protected:
    Expression(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::EXPRESSION;
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

class EndOfFile : public Statement {
public:
    static const IRType ClassType = IRType::END_OF_FILE;

    EndOfFile() : Statement(ClassType, "<end of file>") {}
};

/**
 * Types of operators
 */
enum OperatorKind {
    OP_CONCAT, ///< ++
    OP_EXP,    ///< ^
    OP_PLUS,   ///< `+`
    OP_MINUS,  ///< `-`
    OP_DIV,    ///< /
    OP_MUL,    ///< `*`
    OP_MOD,    ///< %
    OP_SET,    ///< =
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
    OP_SLICE,  ///< [..]
    OP_ACCESS, ///< `.`
    OP_SUBSC,  ///< []

    //OP_UNKNOWN
};

/**
 * Operator in an expression
 */
class Operator {
private:
    OperatorKind kind;
public:
    Operator(OperatorKind kind) : kind(kind) {}

    OperatorKind getKind() const { return kind; }

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
        case OP_SLICE: os << "[..]"; break;
        case OP_ACCESS: os << "."; break;
        case OP_SUBSC: os << "[]"; break;
        //case OP_UNKNOWN: os << "unknown"; break;
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

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << *left << " " << op << " " << *right;
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
        os << utils::sanitize(value);
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