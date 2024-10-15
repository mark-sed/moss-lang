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
#include "utils.hpp"
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
    ADDRESS,
    FUN,
    FUN_LIST,
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
    virtual ~Value() {
        for (auto [_, v] : attrs) {
            delete v;
        }
    }

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

class AddrValue : public Value {
private:
    opcode::Address value;
public:
    static const TypeKind ClassType = TypeKind::ADDRESS;

    AddrValue(opcode::Address value) : Value(ClassType, "<address>"), value(value) {}
    virtual Value *clone() {
        return new AddrValue(this->value);
    }

    opcode::Address get_value() { return this->value; }

    virtual opcode::StringConst as_string() override {
        assert(false && "Getting address value as a string");
        return name;
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Addr(" << value << ")[refs: " << references << "]";
        return os;
    }
};

class FunValueArg {
public:
    opcode::StringConst name;
    std::vector<opcode::StringConst> types;
    Value *default_value;
    bool vararg;

    FunValueArg(opcode::StringConst name,
                std::vector<opcode::StringConst> types,
                Value *default_value=nullptr,
                bool vararg=false) 
               : name(name), types(types), default_value(default_value), vararg(vararg) {}

    // TODO: Debug
};

class FunValue : public Value {
private:
    opcode::StringConst name;
    std::vector<FunValueArg *> args;
    opcode::Address body_addr;
public:
    static const TypeKind ClassType = TypeKind::FUN;

    FunValue(opcode::StringConst name, opcode::StringConst arg_names) 
            : Value(ClassType, "Function"), name(name), args(), body_addr(0) {
        auto names = utils::split_csv(arg_names, ',');
        for (auto n: names) {
            args.push_back(new FunValueArg(n, std::vector<opcode::StringConst>{}));
        }
    }

    FunValue(opcode::StringConst name,
             std::vector<FunValueArg *> args,
             opcode::Address body_addr) 
            : Value(ClassType, "Function"), name(name), args(args), body_addr(body_addr) {}

    virtual Value *clone() {
        return new FunValue(this->name, this->args, this->body_addr);
    }

    void set_vararg(opcode::IntConst index) {
        assert(index < static_cast<int>(args.size()) && "out of bounds argument");
        args[index]->vararg = true;
    }

    void set_type(opcode::IntConst index, opcode::StringConst type) {
        assert(index < static_cast<int>(args.size()) && "out of bounds argument");
        args[index]->types.push_back(type);
    }

    void set_default(opcode::IntConst index, Value *v) {
        assert(index < static_cast<int>(args.size()) && "out of bounds argument");
        args[index]->default_value = v;
    }

    void set_body_addr(opcode::Address body_addr) {
        this->body_addr = body_addr;
    }

    opcode::Address get_body_addr() { return this->body_addr; }

    virtual opcode::StringConst as_string() override {
        return "<function " + name + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Fun(" << name << " @" << body_addr << ")[refs: " << references << "]";
        return os;
    }
};

class FunValueList : public Value {
private:
    std::vector<FunValue *> funs;
public:
    static const TypeKind ClassType = TypeKind::FUN_LIST;

    FunValueList(FunValue *f) : Value(ClassType, "FunctionList") {
        funs.push_back(f);
    }
    FunValueList(std::vector<FunValue *> funs) : Value(ClassType, "FunctionList"), funs(funs) {}
    
    virtual Value *clone() {
        return new FunValueList(this->funs);
    }

    std::vector<FunValue *> get_funs() { return this->funs; }
    void push_back(FunValue *f) { this->funs.push_back(f); }
    FunValue *back() {
        assert(!funs.empty() && "no functions in funlist");
        return funs.back();
    }

    virtual opcode::StringConst as_string() override {
        assert(!funs.empty() && "sanity check");
        return "<functions " + funs[0]->get_name() + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "FunValueList(";
        bool first = true;
        for (auto f: funs) {
            if (first) {
                os << *f;
                first = false;
            }
            else {
                os << ", " << *f;
            }
        }
        os << ")[refs: " << references << "]";
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