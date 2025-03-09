/// 
/// \file values.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss VM types
/// 

#ifndef _VALUES_HPP_
#define _VALUES_HPP_

#include "commons.hpp"
#include "utils.hpp"
#include "memory.hpp"
#include "clopts.hpp"
#include "builtins.hpp"
#include <cstdint>
#include <map>
#include <sstream>
#include <list>
#include <vector>

#include "logging.hpp"

namespace moss {

class MemoryPool;
class Interpreter;

/// \note Add any new that have object methods to has_methods
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
    OBJECT,
    CLASS,
    MODULE,
    SPACE,
    ENUM,
    ENUM_VALUE,

    CPP_FSTREAM
};

inline ustring TypeKind2String(TypeKind kind) {
    switch(kind) {
        case TypeKind::INT: return "INT";
        case TypeKind::FLOAT: return "FLOAT";
        case TypeKind::BOOL: return "BOOL";
        case TypeKind::NIL: return "NIL";
        case TypeKind::STRING: return "STRING";
        case TypeKind::LIST: return "LIST";
        case TypeKind::DICT: return "DICT";
        case TypeKind::ADDRESS: return "ADDRESS";
        case TypeKind::FUN: return "FUN";
        case TypeKind::FUN_LIST: return "FUN_LIST";
        case TypeKind::OBJECT: return "OBJECT";
        case TypeKind::CLASS: return "CLASS";
        case TypeKind::MODULE: return "MODULE";
        case TypeKind::SPACE: return "SPACE";
        case TypeKind::ENUM: return "ENUM";
        case TypeKind::ENUM_VALUE: return "ENUM_VALUE";
        case TypeKind::CPP_FSTREAM: return "CPP_FSTREAM";
    }
    assert(false && "Type kind in to string conversion");
    return "UNKNOWN";
}

/// Base class of all values
class Value {
protected:
    bool marked;
    
    TypeKind kind;
    Value *type;
    ustring name;
    
    MemoryPool *attrs;
    std::map<ustring, Value *> annotations;

    Value(TypeKind kind, ustring name, Value *type, MemoryPool *attrs=nullptr);

    static int tab_depth;
public:
    virtual Value *clone() = 0;
    virtual ~Value();

    static std::list<Value *> all_values;
    static size_t allocated_bytes;
    static size_t next_gc;

    /// We need to store any allocation to all object list for GC to collect it
    /// once not used
    void *operator new(size_t size);

    static void operator delete(void * p, size_t size);

    void set_marked(bool m) { this->marked = m; }
    bool is_marked() { return this->marked; }

    TypeKind get_kind() { return this->kind; }
    Value *get_type() { return this->type; }
    ustring get_name() { return this->name; }

    /// When mutable, then the value can have attributes assigned into
    virtual inline bool is_mutable() { return false; }

    virtual std::ostream& debug(std::ostream& os) const = 0;

    virtual opcode::StringConst as_string() const = 0;
    virtual opcode::StringConst dump() {
        return as_string();
    }
    virtual opcode::FloatConst as_float() {
        // FIXME: raise error
        assert(false && "as_float requested on non numerical value");
        return 0.0;
    }
    virtual Value *iter(Interpreter *vm);
    virtual Value *next(Interpreter *vm);

    void annotate(ustring name, Value *val);
    std::map<ustring, Value *> get_annotations() { return this->annotations; }
    bool has_annotation(ustring name) {
        return annotations.find(name) != annotations.end();
    }
    Value *get_annotation(ustring name) {
        assert(has_annotation(name) && "Did not check annotation existence");
        return annotations[name];
    }
 
    /// Returns register in which is attribute stored 
    /// If this attribute is not set, then nullptr is returned
    /// \param name Attribute name
    /// \return Value of attribute or nullptr if not set
    Value *get_attr(ustring name);
    bool has_attr(ustring name) { return get_attr(name) != nullptr; }

    /// Sets (new or overrides) attribute name to value v
    void set_attr(ustring name, Value *v);

    void set_attrs(MemoryPool *p);
    void copy_attrs(MemoryPool *p);
    MemoryPool *get_attrs() { return this->attrs; }
};

inline std::ostream& operator<< (std::ostream& os, Value &v) {
    return v.debug(os);
}

bool has_methods(Value *v);

// Forward declarations
template<class T>
bool isa(Value& t);

template<class T>
bool isa(Value* t);

template<class T>
T *dyn_cast(Value* t);

/// Moss integer value
class IntValue : public Value {
private:
    opcode::IntConst value;
public:
    static const TypeKind ClassType = TypeKind::INT;

    IntValue(opcode::IntConst value);

    virtual Value *clone() {
        return new IntValue(this->value);
    }

    opcode::IntConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() const override {
        return std::to_string(value);
    }

    virtual opcode::FloatConst as_float() override {
        return static_cast<opcode::FloatConst>(value);
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Int(" << value << ")";
        return os;
    }
};

/// Moss float value
class FloatValue : public Value {
private:
    opcode::FloatConst value;
public:
    static const TypeKind ClassType = TypeKind::FLOAT;

    FloatValue(opcode::FloatConst value);
    
    virtual Value *clone() {
        return new FloatValue(this->value);
    }

    opcode::FloatConst get_value() { return this->value; }
    virtual opcode::FloatConst as_float() override {
        return this->value;
    }

    virtual opcode::StringConst as_string() const override {
        return std::to_string(value);
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Float(" << value << ")";
        return os;
    }
};

/// Moss boolean value
class BoolValue : public Value {
private:
    opcode::BoolConst value;
public:
    static const TypeKind ClassType = TypeKind::BOOL;

    BoolValue(opcode::BoolConst value);
    
    virtual Value *clone() {
        return new BoolValue(this->value);
    }

    opcode::BoolConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() const override {
        return value ? "true" : "false";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Bool(" << (value ? "true" : "false") << ")";
        return os;
    }
};

/// Moss string value
class StringValue : public Value {
private:
    opcode::StringConst value;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::STRING;

    StringValue(opcode::StringConst value);

    virtual Value *clone() {
        return new StringValue(this->value);
    }

    opcode::StringConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() const override {
        return value;
    }

    virtual Value *iter(Interpreter *vm) override {
        (void)vm;
        iterator = 0;
        return this;
    }

    virtual Value *next(Interpreter *vm) override;

    virtual opcode::StringConst dump() override {
        return "\"" + value + "\"";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "String(\"" << utils::sanitize(value) << "\")";
        return os;
    }
};

/// Moss nil value (holds only one value)
class NilValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::NIL;

    NilValue() : Value(ClassType, "Nil", BuiltIns::NilType) {}
    virtual Value *clone() {
        return new NilValue();
    }

    virtual opcode::StringConst as_string() const override {
        return "nil";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "NilType(nil)";
        return os;
    }
};

class ListValue : public Value {
private:
    std::vector<Value *> vals;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::LIST;

    ListValue(std::vector<Value *> vals);
    ListValue();

    virtual Value *clone() {
        return new ListValue(this->vals);
    }

    std::vector<Value *> get_vals() { return this->vals; }

    void push(Value *v) { vals.push_back(v); }

    virtual opcode::StringConst as_string() const override {
        if (vals.empty()) return "[]";
        std::stringstream ss;
        ss << "[";
        bool first = true;
        for (auto v: vals) {
            if (first) {
                ss << v->dump();
                first = false;
            }
            else {
                ss << ", " << v->dump();
            }
        }
        ss << "]";

        return ss.str();
    }

    virtual Value *iter(Interpreter *vm) override {
        (void)vm;
        iterator = 0;
        return this;
    }

    virtual Value *next(Interpreter *vm) override;

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "List(" << vals.size() << ") [";
        if (vals.empty()) {
            os << "]";
            return os;
        }

        bool first = true;
        ++tab_depth;
        for (auto v: vals) {
            // TODO: tab has to increase with each use so that if
            // list or some other structure is outputted it will also have
            // correct tabs.
            os << (first ? "" : ",") << "\n" << std::string(tab_depth*2, ' ') << *v; 
            first = false;
        }
        --tab_depth;
        os << "\n" << std::string(tab_depth*2, ' ') << "]";
        return os;
    }
};

class ClassValue : public Value {
private:
    std::list<ClassValue *> supers;
public:
    static const TypeKind ClassType = TypeKind::CLASS;

    ClassValue(ustring name) : Value(ClassType, name, BuiltIns::Type) {}
    ClassValue(ustring name, std::list<ClassValue *> supers) 
        : Value(ClassType, name, BuiltIns::Type), supers(supers) {}
    ClassValue(ustring name, MemoryPool *frm, std::list<ClassValue *> supers) 
        : Value(ClassType, name, BuiltIns::Type, frm), supers(supers) {}

    virtual Value *clone() {
        auto cpy = new ClassValue(this->name, this->supers);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }

    void bind(ClassValue *cls) {
        set_attrs(cls->get_attrs());
        this->supers = cls->get_supers();
        this->annotations = cls->get_annotations();
    }

    virtual inline bool is_mutable() override { return true; }

    virtual opcode::StringConst as_string() const override {
        return "<class " + name + ">";
    }

    std::list<ClassValue *> get_supers() { return this->supers; }
    
    bool has_parent(ClassValue *c) {
        return std::find(supers.begin(), supers.end(), c) != supers.end();
    }

    std::list<ClassValue *> get_all_supers();

    virtual std::ostream& debug(std::ostream& os) const override;
};

class ObjectValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::OBJECT;

    ObjectValue(ClassValue *cls) : Value(ClassType, "<object>", cls) {
        this->copy_attrs(cls->get_attrs());
    }

    virtual Value *clone() {
        assert(isa<ClassValue>(this->type) && "type was modified?");
        auto cpy = new ObjectValue(dyn_cast<ClassValue>(this->type));
        cpy->copy_attrs(this->get_attrs());
        return cpy;
    }

    virtual inline bool is_mutable() override { return true; }

    virtual opcode::StringConst as_string() const override {
        return "<object of class " + this->type->get_name() + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override;
};

class SpaceValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::SPACE;

    SpaceValue(ustring name) : Value(ClassType, name, BuiltIns::Space) {}
    SpaceValue(ustring name, MemoryPool *frm) : Value(ClassType, name, BuiltIns::Space, frm) {}
    ~SpaceValue() {}

    virtual Value *clone() {
        auto cpy = new SpaceValue(this->name);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }

    virtual inline bool is_mutable() override { return true; }

    virtual opcode::StringConst as_string() const override {
        return "<space " + name + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override;
};

class ModuleValue : public Value {
private:
    Interpreter *vm;
public:
    static const TypeKind ClassType = TypeKind::MODULE;

    ModuleValue(ustring name, Interpreter *vm)
        : Value(ClassType, name, BuiltIns::Module), vm(vm) {}
    ModuleValue(ustring name, MemoryPool *frm, Interpreter *vm)
        : Value(ClassType, name, BuiltIns::Module, frm), vm(vm) {}
    ~ModuleValue();

    virtual Value *clone() {
        auto cpy = new ModuleValue(this->name, this->vm);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }

    virtual opcode::StringConst as_string() const override {
        return "<module " + name + ">";
    }

    Interpreter *get_vm() { return vm; }

    virtual std::ostream& debug(std::ostream& os) const override;
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

    bool is_typed() { return types.empty(); }
    // TODO: Debug
};

class FunValue : public Value {
private:
    std::vector<FunValueArg *> args;
    std::list<MemoryPool *> closures;
    Interpreter *vm;
    opcode::Address body_addr;
public:
    static const TypeKind ClassType = TypeKind::FUN;

    FunValue(opcode::StringConst name, opcode::StringConst arg_names, Interpreter *vm) 
            : Value(ClassType, name, BuiltIns::Function), 
              args(), closures(), vm(vm), body_addr(0) {
        auto names = utils::split_csv(arg_names, ',');
        for (auto n: names) {
            args.push_back(new FunValueArg(n, std::vector<Value *>{}));
        }
    }

    FunValue(opcode::StringConst name,
             std::vector<FunValueArg *> args,
             Interpreter *vm,
             opcode::Address body_addr) 
            : Value(ClassType, name, BuiltIns::Function), args(args), vm(vm), body_addr(body_addr) {}

    ~FunValue() {
        for(auto a: args)
            delete a;
    }

    virtual Value *clone() {
        return new FunValue(this->name, this->args, this->vm, this->body_addr);
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

    void push_closure(MemoryPool *p) {
        closures.push_back(p);
    }

    std::list<MemoryPool *> get_closures() {
        return this->closures;
    }

    opcode::Address get_body_addr() { return this->body_addr; }

    std::vector<FunValueArg *> get_args() { return this->args; }

    Interpreter *get_vm() { return this->vm; }

    ustring get_args_as_str() {
        std::stringstream os;
        bool first = true;
        for (auto a: args) {
            if (!first) {
                os << ", ";
            }
            first = false;
            os << a->name;
            auto types = a->types;
            if (!types.empty()) {
                os << ":[";
                bool first_type = true;
                for (auto t : types) {
                    if (!first_type)
                        os << ", ";
                    first_type = false;
                    os << t->get_name();
                }
                os << "]";
            }
        }
        return os.str();
    }

    virtual opcode::StringConst as_string() const override {
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
        os << ")";
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

    virtual opcode::StringConst as_string() const override {
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
        os << ")";
        return os;
    }
};

class EnumTypeValue;

class EnumValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::ENUM_VALUE;

    EnumValue(EnumTypeValue *type, ustring name);

    virtual Value *clone();

    virtual opcode::StringConst as_string() const override {
        return name;
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << type->get_name() << "(" << name << ")";
        return os;
    }
};

class EnumTypeValue : public Value {
private:
    std::vector<EnumValue *> vals;
public:
    static const TypeKind ClassType = TypeKind::ENUM;

    EnumTypeValue(ustring name) : Value(ClassType, name, BuiltIns::Type) {}
    EnumTypeValue(ustring name, std::vector<EnumValue *> vals) 
        : Value(ClassType, name, BuiltIns::Type), vals(vals) {}

    virtual Value *clone() {
        return new EnumTypeValue(this->name, this->vals);
    }

    void set_values(std::vector<EnumValue *> vals) {
        this->vals = vals;
    }
    std::vector<EnumValue *> get_values() {
        return this->vals;
    }

    virtual opcode::StringConst as_string() const override {
        return "<Enum " + name + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Enum {";
        if (vals.empty()) {
            os << "}";
        }
        else {
            bool first = true;
            ++tab_depth;
            for (auto v: vals) {
                if (!first) {
                    os << ",";
                }
                first = false;
                os << "\n";
                os << std::string(tab_depth*2, ' ') << v->get_name();
            }
            --tab_depth;
            os << "\n" << std::string(tab_depth*2, ' ') << "}";
        }
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