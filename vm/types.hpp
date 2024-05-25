/**
 * @file types.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Moss VM types
 */

#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include "os_interface.hpp"
#include <cstdint>

namespace moss {

namespace types {

enum class TypeKind {
    INT,
    FLOAT,
    BOOL,
    STRING,
    LIST,
    DICT,

    USER_DEF
};

/** Base type of all types, its type is itself */
class Type {
protected:
    TypeKind kind;
    ustring name;
    
    Type(TypeKind kind, ustring name) : kind(kind), name(name) {}
public:
    virtual ~Type() {}

    TypeKind get_kind() { return this->kind; }
    ustring get_name() { return this->name; }
};

class Int : public Type {
private:
    int64_t value;
public:
    static const TypeKind ClassType = TypeKind::INT;

    Int(int64_t value) : Type(ClassType, "Int"), value(value) {}

    int64_t get_value() { return this->value; }
};


}

// Helper functions
template<class T>
bool isa(types::Type& t) {
    return t.get_kind() == T::ClassType;
}

template<class T>
bool isa(types::Type* t) {
    return t->get_kind() == T::ClassType;
}

template<class T>
T *dyn_cast(types::Type* t) {
    if (!isa<T>(t)) return nullptr;
    return dynamic_cast<T *>(t);
}

}

#endif//_TYPES_HPP_