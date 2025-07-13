/// 
/// \file values.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
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
#include <algorithm>
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
    NOTE,
    FUN,
    FUN_LIST,
    OBJECT,
    CLASS,
    MODULE,
    SPACE,
    ENUM,
    ENUM_VALUE,
    SUPER_VALUE,

    CPP_FSTREAM,
    CPP_VOID_STAR,
    CPP_FFI_CIF
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
        case TypeKind::NOTE: return "NOTE";
        case TypeKind::FUN: return "FUN";
        case TypeKind::FUN_LIST: return "FUN_LIST";
        case TypeKind::OBJECT: return "OBJECT";
        case TypeKind::CLASS: return "CLASS";
        case TypeKind::MODULE: return "MODULE";
        case TypeKind::SPACE: return "SPACE";
        case TypeKind::ENUM: return "ENUM";
        case TypeKind::ENUM_VALUE: return "ENUM_VALUE";
        case TypeKind::SUPER_VALUE: return "SUPER_VALUE";
        case TypeKind::CPP_FSTREAM: return "CPP_FSTREAM";
        case TypeKind::CPP_VOID_STAR: return "CPP_VOID_STAR";
        case TypeKind::CPP_FFI_CIF: return "CPP_FFI_CIF";
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
#ifndef NDEBUG
    static long allocated;
#endif
    virtual Value *clone() = 0;
    virtual ~Value();

    static std::list<Value *> all_values; ///< Holds all ever allocated values (for GC)
    static size_t allocated_bytes; ///< Number of currently allocated bytes for values (this lowers with delete)
    static size_t next_gc; ///< Threshold in bytes for next GC run

    /// We need to store any allocation to all object list for GC to collect it
    /// once not used
    void *operator new(size_t size);

    /// Custom delete operator for adjusting the allocated_bytes number
    static void operator delete(void * p, size_t size);

    void set_marked(bool m) { this->marked = m; }
    bool is_marked() { return this->marked; }

    TypeKind get_kind() { return this->kind; }
    Value *get_type() { return this->type; }
    ustring get_name() { return this->name; }

    /// When modifiable, then the value can have attributes assigned into
    virtual inline bool is_modifiable() { return false; }
    /// When mutable, then the value can change
    virtual inline bool is_hashable() = 0;
    virtual inline bool is_iterable() { return false; }

    virtual std::ostream& debug(std::ostream& os) const = 0;

    virtual opcode::StringConst as_string() const = 0;
    virtual opcode::StringConst dump() {
        return as_string();
    }
    /// Converts numeric type to float. The type has to be checked if it is
    /// numeric otherwise assert is raised or 0.0 returned
    virtual opcode::FloatConst as_float() {
        // FIXME: raise error
        assert(false && "as_float requested on non numerical value");
        return 0.0;
    }
    /// Moss's __iter method for the value
    virtual Value *iter(Interpreter *vm);
    /// Moss's __next method for the value
    virtual Value *next(Interpreter *vm);
    /// Set of indexed values
    virtual void set_subsc(Interpreter *vm, Value *obj, Value *val);
    /// Hash of the value
    virtual opcode::IntConst hash() {
        assert(!is_hashable() && "Calling hash on non-hashable type");
        assert(false && "Missing hash implementation");
        return 0;
    }

    /// Adds an annotation to the value (if possible)
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
    /// \param owner_vm VM which does this call
    /// \return Value of attribute or nullptr if not set
    virtual Value *get_attr(ustring name, Interpreter *caller_vm);
    bool has_attr(ustring name, Interpreter *caller_vm) { return get_attr(name, caller_vm) != nullptr; }

    /// Sets (new or overrides) attribute name to value v
    void set_attr(ustring name, Value *v, bool internal_access=false);

    /// Removes and attribute
    bool del_attr(ustring name, Interpreter *vm);

    void set_attrs(MemoryPool *p);
    void copy_attrs(MemoryPool *p);
    MemoryPool *get_attrs() { return this->attrs; }
};

inline std::ostream& operator<< (std::ostream& os, Value &v) {
    return v.debug(os);
}

bool has_methods(Value *v);
opcode::IntConst hash(Value *v, Interpreter *vm);

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

    virtual Value *clone() override {
        return new IntValue(this->value);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return value;
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
    
    virtual Value *clone() override {
        return new FloatValue(this->value);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<opcode::FloatConst>{}(value);
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
    
    virtual Value *clone() override {
        return new BoolValue(this->value);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return value ? 1L : 0L;
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
protected:
    opcode::StringConst value;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::STRING;

    StringValue(opcode::StringConst value);

    virtual Value *clone() override {
        return new StringValue(this->value);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<opcode::StringConst>{}(value);
    }
    virtual inline bool is_iterable() override { return true; }

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

    virtual Value *clone() override {
        return new NilValue();
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return -1L;
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

    virtual Value *clone() override {
        return new ListValue(this->vals);
    }

    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; }

    std::vector<Value *> get_vals() { return this->vals; }

    void push(Value *v) { vals.push_back(v); }
    size_t size() { return vals.size(); }
    void remove(long i) { vals.erase(vals.begin() + i); }

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
    virtual void set_subsc(Interpreter *vm, Value *key, Value *val) override;

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

class DictValue : public Value {
private:
    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> vals;    
    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>>::iterator iterator;
    size_t keys_iterator;
public:
    static const TypeKind ClassType = TypeKind::DICT;

    DictValue(ListValue *keys, ListValue *values, Interpreter *vm);
    DictValue(std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> vals);
    DictValue();

    ~DictValue() {}

    virtual Value *clone() override {
        return new DictValue(this->vals);
    }

    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; }

    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> get_vals() { return this->vals; }

    void push(Value *k, Value *v, Interpreter *vm);
    size_t size() { return vals.size(); }

    virtual opcode::StringConst as_string() const override {
        if (vals.empty()) return "{:}";
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (auto [k, v]: vals) {
            for (auto vl: v) {
                if (first) {
                    ss << vl.first->dump() << ": " << vl.second->dump();
                    first = false;
                }
                else {
                    ss << ", " << vl.first->dump() << ": " << vl.second->dump();
                }
            }
        }
        ss << "}";

        return ss.str();
    }

    virtual Value *iter(Interpreter *vm) override {
        (void)vm;
        iterator = vals.begin();
        keys_iterator = 0;
        return this;
    }

    virtual Value *next(Interpreter *vm) override;
    virtual void set_subsc(Interpreter *vm, Value *key, Value *val) override;

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Dict(" << vals.size() << ") {";
        if (vals.empty()) {
            os << "}";
            return os;
        }

        bool first = true;
        ++tab_depth;
        for (auto [k, v]: vals) {
            // TODO: tab has to increase with each use so that if
            // list or some other structure is outputted it will also have
            // correct tabs.
            for (auto vl: v) {
                os << (first ? "" : ",") << "\n" << std::string(tab_depth*2, ' ') 
                << "(" << k << ")" << *vl.first << ": " << *vl.second; 
                first = false;
            }
        }
        --tab_depth;
        os << "\n" << std::string(tab_depth*2, ' ') << "}";
        return os;
    }
};

class ClassValue : public Value {
private:
    std::list<ClassValue *> supers;
public:
    static const TypeKind ClassType = TypeKind::CLASS;

    // If this is called by Type then Type is nullptr, so set type to this
    ClassValue(ustring name) : Value(ClassType, name, BuiltIns::Type ? BuiltIns::Type : this) {}
    ClassValue(ustring name, std::list<ClassValue *> supers) 
        : Value(ClassType, name,  BuiltIns::Type ? BuiltIns::Type : this), supers(supers) {}
    ClassValue(ustring name, MemoryPool *frm, std::list<ClassValue *> supers) 
        : Value(ClassType, name, (BuiltIns::Type ? BuiltIns::Type : this), frm), supers(supers) {}
    ~ClassValue() {}

    virtual Value *clone() override {
        auto cpy = new ClassValue(this->name, this->supers);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0c_"+name);
    }

    void bind(ClassValue *cls) {
        set_attrs(cls->get_attrs());
        this->supers = cls->get_supers();
        this->annotations = cls->get_annotations();
    }

    virtual inline bool is_modifiable() override { return true; }

    virtual opcode::StringConst as_string() const override {
        return "<class " + name + ">";
    }

    std::list<ClassValue *> get_supers() { return this->supers; }
    void push_parent(ClassValue *c) {
        this->supers.push_back(c);
    }
    
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

    virtual Value *clone() override {
        assert(isa<ClassValue>(this->type) && "type was modified?");
        auto cpy = new ObjectValue(dyn_cast<ClassValue>(this->type));
        cpy->copy_attrs(this->get_attrs());
        return cpy;
    }

    virtual Value *iter(Interpreter *vm) override;
    virtual Value *next(Interpreter *vm) override;

    virtual inline bool is_modifiable() override { return true; }
    virtual inline bool is_hashable() override { return true; }
    virtual inline bool is_iterable() override { return true; }

    virtual opcode::StringConst as_string() const override {
        return "<object of class " + this->type->get_name() + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override;
};

class SpaceValue : public Value {
private:
    Interpreter *owner_vm;
    bool anonymous;
public:
    static const TypeKind ClassType = TypeKind::SPACE;

    SpaceValue(ustring name, Interpreter *owner_vm, bool anonymous=false)
        : Value(ClassType, name, BuiltIns::Space), owner_vm(owner_vm), anonymous(anonymous) {}
    SpaceValue(ustring name, MemoryPool *frm, Interpreter *owner_vm, bool anonymous=false)
        : Value(ClassType, name, BuiltIns::Space, frm), owner_vm(owner_vm), anonymous(anonymous) {}
    ~SpaceValue() {}

    virtual Value *clone() override {
        auto cpy = new SpaceValue(this->name, owner_vm, anonymous);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }
    
    Interpreter *get_owner_vm() { return this->owner_vm; }
    bool is_anonymous() { return this->anonymous; }
    
    virtual inline bool is_modifiable() override { return true; }
    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0s_"+name);
    }

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

    virtual Value *clone() override {
        auto cpy = new ModuleValue(this->name, this->vm);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0m_"+name);
    }

    virtual opcode::StringConst as_string() const override {
        return "<module " + name + ">";
    }

    Interpreter *get_vm() { return vm; }

    virtual std::ostream& debug(std::ostream& os) const override;
};

class NoteValue : public StringValue{
private:
    opcode::StringConst format;

public:
    static const TypeKind ClassType = TypeKind::NOTE;

    NoteValue(opcode::StringConst format, StringValue *value);

    virtual Value *clone() override {
        return new NoteValue(this->format, dyn_cast<StringValue>(this));
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<opcode::StringConst>{}(format+value);
    }
    virtual inline bool is_iterable() override { return true; }

    opcode::StringConst get_format() { return this->format; }

    virtual opcode::StringConst dump() override {
        return format + dyn_cast<StringValue>(this)->dump();
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Note(" << format << "\"" << utils::sanitize(value) << "\")";
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

    ~FunValue();

    virtual Value *clone() override {
        return new FunValue(this->name, this->args, this->vm, this->body_addr);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0f_"+name);
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

    ustring get_args_as_str() const {
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

    /// Checks if this function matches by name and arugments other function
    bool equals(FunValue *other) {
        if (args.size() != other->args.size()) {
            return false;
        }
        for (size_t i = 0; i < args.size(); ++i) {
            auto a1 = args[i];
            auto a2 = other->args[i];
            if (a1->vararg || a2->vararg) {
                if (a1->vararg != a2->vararg) {
                    return false;
                }
                continue;
            }
            if (a1->types.size() != a2->types.size()) {
                return false;
            }
            // We care only for the type/untyped and if it is a vararg
            if(!std::is_permutation(a1->types.begin(), a1->types.end(), a2->types.begin())) {
                return false;
            }
        }
        return true;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<function " << name << " at " << std::hex << static_cast<const void*>(this) << ">";
        return ss.str();
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Fun(" << name << "(" << get_args_as_str() << ") @" << body_addr;
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
    
    virtual Value *clone() override {
        return new FunValueList(this->funs);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0fl_"+name);
    }

    std::vector<FunValue *> &get_funs() { return this->funs; }
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

    virtual Value *clone() override;

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0ev_"+name);
    }

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

    virtual Value *clone() override {
        return new EnumTypeValue(this->name, this->vals);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0e_"+name);
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

class SuperValue : public Value {
private:
    ObjectValue *instance;
public:
    static const TypeKind ClassType = TypeKind::SUPER_VALUE;

    SuperValue(ObjectValue *instance)
        : Value(ClassType, "super", BuiltIns::super), instance(instance) {}
    ~SuperValue() {}

    virtual Value *clone() override {
        return new SuperValue(this->instance);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0sp_"+name);
    }

    ObjectValue *get_instance() { return this->instance; }

    virtual Value *get_attr(ustring name, Interpreter *caller_vm) override;

    virtual opcode::StringConst as_string() const override {
        return "<super of " + instance->get_type()->get_name() + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "super(" << *instance << ")";
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
    assert(t && "Passed nullptr to isa");
    return t->get_kind() == T::ClassType;
}

template<class T>
T *dyn_cast(Value* t) {
    assert(t && "Passed nullptr to dyn_cast");
    if constexpr (std::is_same_v<T, StringValue>) {
        // special case for StringValue when NoteValue (its child) is passed in
        if (!isa<StringValue>(t) && !isa<NoteValue>(t)) return nullptr;
        return dynamic_cast<StringValue*>(t);
    } else {
        if (!isa<T>(t)) return nullptr;
        return dynamic_cast<T *>(t);
    }
}

}

#endif//VALUES_HPP_