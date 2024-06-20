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

class IntValue : public Value {
private:
    int64_t value;
public:
    static const TypeKind ClassType = TypeKind::INT;

    IntValue(int64_t value) : Value(ClassType, "Int"), value(value) {}
    virtual Value *clone() {
        return new IntValue(this->value);
    }

    int64_t get_value() { return this->value; }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Int(" << value << ")[refs: " << references << "]";
        return os;
    }
};

class FloatValue : public Value {
private:
    double value;
public:
    static const TypeKind ClassType = TypeKind::FLOAT;

    FloatValue(double value) : Value(ClassType, "Float"), value(value) {}
    virtual Value *clone() {
        return new FloatValue(this->value);
    }

    double get_value() { return this->value; }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Float(" << value << ")[refs: " << references << "]";
        return os;
    }
};

class BoolValue : public Value {
private:
    bool value;
public:
    static const TypeKind ClassType = TypeKind::BOOL;

    BoolValue(bool value) : Value(ClassType, "Bool"), value(value) {}
    virtual Value *clone() {
        return new BoolValue(this->value);
    }

    bool get_value() { return this->value; }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Bool(" << (value ? "true" : "false") << ")[refs: " << references << "]";
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