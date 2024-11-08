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
#include <sstream>

namespace moss {

enum class TypeKind {
    // Primitive types
    INT,
    FLOAT,
    BOOL,
    NIL,
    STRING,
    LIST,
    DICT,
    ADDRESS,
    FUN,    // Used as a stopper for primitive types in opcode
    // Non-primitive types
    FUN_LIST,
    OBJECT,
    CLASS
};

/** Base class of all values */
class Value {
protected:
    int references;
    
    TypeKind kind;
    Value *type;
    ustring name;
    
    std::map<ustring, Value *> attrs;
    std::map<ustring, Value *> annotations;

    Value(TypeKind kind, ustring name, Value *type) 
        : references(1), kind(kind), type(type), name(name), attrs(), annotations{} {}
public:
    virtual Value *clone() = 0;
    virtual ~Value() {
        // We cannot just delete the value of annotation as it might be class name
        // or used somewhere else, let gc handle it
    }

    virtual opcode::StringConst as_string() = 0;
    virtual opcode::FloatConst as_float() {
        // FIXME: raise error
        assert(false && "as_float requested on non numerical value");
        return 0.0;
    }

    TypeKind get_kind() { return this->kind; }
    Value *get_type() { return this->type; }
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

    void annotate(ustring name, Value *val) {
        annotations[name] = val;
    }
    std::map<ustring, Value *> get_annotations() { return this->annotations; }
    bool has_annotation(ustring name) {
        return annotations.find(name) != annotations.end();
    }
    Value *get_annotation(ustring name) {
        assert(has_annotation(name) && "Did not check annotation existence");
        return annotations[name];
    }

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

/**
 * This namespace contains values (pointers) for all the built-in types
 * 
 * NOTE: When adding a new value also add it to Interpreter::init_global_frame
 */
namespace BuiltIns {
    extern Value *Type;// = new ClassValue("Type");
    extern Value *Int;// = new ClassValue("Int");
    extern Value *Float;// = new ClassValue("Float");
    extern Value *Bool;// = new ClassValue("Bool");
    extern Value *NilType;// = new ClassValue("NilType");
    extern Value *String;// = new ClassValue("String");
    extern Value *List;
    extern Value *Address;// = new ClassValue("Address");
    extern Value *Function;// = new ClassValue("Function");
    extern Value *FunctionList;// = new ClassValue("FunctionList");

    extern Value *Nil;
    #define BUILT_INS_INT_CONSTANTS_AM 262
    extern Value *IntConstants[BUILT_INS_INT_CONSTANTS_AM];
}

/** Moss integer value */
class IntValue : public Value {
private:
    opcode::IntConst value;
public:
    static const TypeKind ClassType = TypeKind::INT;

    IntValue(opcode::IntConst value) : Value(ClassType, "Int", BuiltIns::Int), value(value) {}
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

    FloatValue(opcode::FloatConst value) : Value(ClassType, "Float", BuiltIns::Float), value(value) {}
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

    BoolValue(opcode::BoolConst value) : Value(ClassType, "Bool", BuiltIns::Bool), value(value) {}
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

    StringValue(opcode::StringConst value) : Value(ClassType, "String", BuiltIns::String), value(value) {}
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

    NilValue() : Value(ClassType, "Nil", BuiltIns::NilType) {}
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

class ListValue : public Value {
private:
    std::vector<Value *> vals;
public:
    static const TypeKind ClassType = TypeKind::LIST;

    ListValue(std::vector<Value *> vals) : Value(ClassType, "List", BuiltIns::List), vals(vals) {}
    ListValue() : Value(ClassType, "List", BuiltIns::List), vals() {}

    virtual Value *clone() {
        return new ListValue(this->vals);
    }

    std::vector<Value *> get_vals() { return this->vals; }

    virtual opcode::StringConst as_string() override {
        if (vals.empty()) return "[]";
        std::stringstream ss;
        ss << "[";
        bool first = true;
        for (auto v: vals) {
            if (first) {
                ss << v->as_string();
                first = false;
            }
            else {
                ss << ", " << v->as_string();
            }
        }
        ss << "]";

        return ss.str();
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "List(size:" << vals.size() << ")[refs: " << references << "]";
        return os;
    }
};

class ClassValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::CLASS;

    ClassValue(ustring name) : Value(ClassType, name, BuiltIns::Type) {}

    virtual Value *clone() {
        return new ClassValue(this->name);
    }

    virtual opcode::StringConst as_string() override {
        return "<Class " + name + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Class(" << name << ")[refs: " << references << "]";
        return os;
    }
};

class FunValueArg {
public:
    opcode::StringConst name;
    std::vector<Value *> types;
    Value *default_value;
    bool vararg;

    FunValueArg(opcode::StringConst name,
                std::vector<Value *> types,
                Value *default_value=nullptr,
                bool vararg=false) 
               : name(name), types(types), default_value(default_value), vararg(vararg) {}

    // TODO: Debug
};

class FunValue : public Value {
private:
    std::vector<FunValueArg *> args;
    opcode::Address body_addr;
public:
    static const TypeKind ClassType = TypeKind::FUN;

    FunValue(opcode::StringConst name, opcode::StringConst arg_names) 
            : Value(ClassType, name, BuiltIns::Function), args(), body_addr(0) {
        auto names = utils::split_csv(arg_names, ',');
        for (auto n: names) {
            args.push_back(new FunValueArg(n, std::vector<Value *>{}));
        }
    }

    FunValue(opcode::StringConst name,
             std::vector<FunValueArg *> args,
             opcode::Address body_addr) 
            : Value(ClassType, name, BuiltIns::Function), args(args), body_addr(body_addr) {}

    virtual Value *clone() {
        return new FunValue(this->name, this->args, this->body_addr);
    }

    void set_vararg(opcode::IntConst index) {
        assert(index < static_cast<int>(args.size()) && "out of bounds argument");
        args[index]->vararg = true;
    }

    void set_type(opcode::IntConst index, Value *type) {
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

    std::vector<FunValueArg *> get_args() { return this->args; }

    virtual opcode::StringConst as_string() override {
        std::stringstream ss;
        ss << "<function " << name << " at " << std::hex << static_cast<const void*>(this) << ">";
        return ss.str();
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Fun(" << name << " @" << body_addr;
        if (!annotations.empty()) {
            os << " annots[";
            bool first = true;
            for (auto [k, v]: annotations) {
                if (!first) os << ", ";
                os << "\"" << k << "\": " << *v;
                first = false;
            }
            os << "]";
        }
        os << ")[refs: " << references << "]";
        return os;
    }
};

class FunValueList : public Value {
private:
    std::vector<FunValue *> funs;
public:
    static const TypeKind ClassType = TypeKind::FUN_LIST;

    FunValueList(FunValue *f) : Value(ClassType, "FunctionList", BuiltIns::FunctionList) {
        funs.push_back(f);
    }
    FunValueList(std::vector<FunValue *> funs) : Value(ClassType, "FunctionList", BuiltIns::FunctionList), funs(funs) {}
    
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
        return "<function " + funs[0]->get_name() + " with " + std::to_string(funs.size()) + " overloads>";
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