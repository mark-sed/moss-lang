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
#include <iostream>
#include <string>
#include <cassert>

namespace moss {

namespace ir {

enum class IRType {
    CONSTRUCT,
    EXPRESSION,
    STATEMENT,

    MODULE,
    SPACE
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
    Construct(IRType ir_type, ustring name) : IR(ir_type, name) {}
public:
    static const IRType ClassType = IRType::CONSTRUCT;
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

    virtual inline std::ostream& debug(std::ostream& os) const {
        os << "<Module>" << name;
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