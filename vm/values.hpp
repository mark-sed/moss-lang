/**
 * @file values.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Moss VM types
 */

#ifndef _VALUES_HPP_
#define _VALUES_HPP_

#include "os_interface.hpp"
#include <cstdint>
#include <map>

namespace moss {

enum class TypeKind {
    INT,
    FLOAT,
    BOOL,
    NIL,
    STRING,
    LIST,
    DICT,

    USER_DEF
};


/** Base class of all values */
class Value {
protected:
    int references;
    
    TypeKind kind;
    ustring name;
    
    std::map<ustring, Value *> attrs;

    Value(TypeKind kind, ustring name) : references(1), kind(kind), name(name), attrs() {}
public:
    virtual Value *clone() = 0;
    virtual ~Value() {}

    virtual opcode::StringConst as_string() = 0;
    virtual opcode::FloatConst as_float() {
        // FIXME: raise error
        assert(false && "as_float requested on non numerical value");
        return 0.0;
    }

    TypeKind get_kind() { return this->kind; }
    ustring get_name() { return this->name; }

    virtual std::ostream& debug(std::ostream& os) const {
        os << "[Value]" << name;
        return os;
    }

    /** Returns how many references are there for this value */
    int get_references() { return this->references; }
    /** Increments reference counter */
    void inc_refs() { this->references += 1; }
    /** Decrements reference counter */
    void dec_refs() { this->references -= 1; }

    /** 
     * Returns register in which is attribute stored 
     * If this attribute is not set, then nullptr is returned
     * @param name Attribute name
     * @return Value of attribute or nullptr is not set
     */
    Value *get_attr(ustring name);

    /** Sets (new or overrides) attribute name to value v*/
    void set_attr(ustring name, Value *v);
};

inline std::ostream& operator<< (std::ostream& os, Value &v) {
    return v.debug(os);
}

/** Moss integer value */
class IntValue : public Value {
private:
    opcode::IntConst value;
public:
    static const TypeKind ClassType = TypeKind::INT;

    IntValue(opcode::IntConst value) : Value(ClassType, "Int"), value(value) {}
    virtual Value *clone() {
        return new IntValue(this->value);
    }

    opcode::IntConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() override {
        return std::to_string(value);
    }

    virtual opcode::FloatConst as_float() override {
        return static_cast<opcode::FloatConst>(value);
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Int(" << value << ")[refs: " << references << "]";
        return os;
    }
};

/** Moss float value */
class FloatValue : public Value {
private:
    opcode::FloatConst value;
public:
    static const TypeKind ClassType = TypeKind::FLOAT;

    FloatValue(opcode::FloatConst value) : Value(ClassType, "Float"), value(value) {}
    virtual Value *clone() {
        return new FloatValue(this->value);
    }

    opcode::FloatConst get_value() { return this->value; }
    virtual opcode::FloatConst as_float() override {
        return this->value;
    }

    virtual opcode::StringConst as_string() override {
        return std::to_string(value);
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Float(" << value << ")[refs: " << references << "]";
        return os;
    }
};

/** Moss boolean value */
class BoolValue : public Value {
private:
    opcode::BoolConst value;
public:
    static const TypeKind ClassType = TypeKind::BOOL;

    BoolValue(opcode::BoolConst value) : Value(ClassType, "Bool"), value(value) {}
    virtual Value *clone() {
        return new BoolValue(this->value);
    }

    opcode::BoolConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() override {
        return value ? "true" : "false";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Bool(" << (value ? "true" : "false") << ")[refs: " << references << "]";
        return os;
    }
};

/** Moss string value */
class StringValue : public Value {
private:
    opcode::StringConst value;
public:
    static const TypeKind ClassType = TypeKind::STRING;

    StringValue(opcode::StringConst value) : Value(ClassType, "String"), value(value) {}
    virtual Value *clone() {
        return new StringValue(this->value);
    }

    opcode::StringConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() override {
        return value;
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "String(\"" << value << "\")[refs: " << references << "]";
        return os;
    }
};

/** Moss nil value (holds only one value) */
class NilValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::NIL;

    NilValue() : Value(ClassType, "Nil") {}
    virtual Value *clone() {
        return new NilValue();
    }

    virtual opcode::StringConst as_string() override {
        return "nil";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "nil" << "[refs: " << references << "]";
        return os;
    }
};

// Helper functions
template<class T>
bool isa(Value& t) {
    return t.get_kind() == T::ClassType;
}

template<class T>
bool isa(Value* t) {
    return t->get_kind() == T::ClassType;
}

template<class T>
T *dyn_cast(Value* t) {
    if (!isa<T>(t)) return nullptr;
    return dynamic_cast<T *>(t);
}

}

#endif//VALUES_HPP_