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
#include <array>

#include "logging.hpp"

namespace moss {

class MemoryPool;
class Interpreter;
class ModuleValue;

/// \note Add any new that have object methods to has_methods
enum class TypeKind {
    INT,
    FLOAT,
    BOOL,
    NIL,
    STRING,
    LIST,
    DICT,
    BYTES,
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

    LIST_ITER,
    DICT_ITER,
    STRING_ITER,
    BYTES_ITER,
    FUN_LIST_ITER,

    // Values after this has to be CPP values as dyn_cast relies on this.
    CPP_CVOID, // This has to be the first cpp value
    CPP_CVOID_STAR,
    CPP_CLONG,
    CPP_CDOUBLE,
    CPP_CCHAR_STAR,
    CPP_FSTREAM,
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
        case TypeKind::BYTES: return "BYTES";
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

        case TypeKind::LIST_ITER: return "LIST_ITER";
        case TypeKind::DICT_ITER: return "DICT_ITER";
        case TypeKind::STRING_ITER: return "STRING_ITER";
        case TypeKind::BYTES_ITER: return "BYTES_ITER";
        case TypeKind::FUN_LIST_ITER: return "FUN_LIST_ITER";

        case TypeKind::CPP_CVOID: return "CPP_CVOID";
        case TypeKind::CPP_CVOID_STAR: return "CPP_CVOID_STAR";
        case TypeKind::CPP_CLONG: return "CPP_CLONG";
        case TypeKind::CPP_CDOUBLE: return "CPP_CDOUBLE";
        case TypeKind::CPP_CCHAR_STAR: return "CPP_CCHAR_STAR";
        case TypeKind::CPP_FSTREAM: return "CPP_FSTREAM";
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
    ModuleValue *owner; ///< Owner module for GC to know value is relying on its module.

    Value(TypeKind kind, ustring name, Value *type, MemoryPool *attrs=nullptr, ModuleValue *owner=nullptr);

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
    virtual std::ostream& debug(std::ostream& os, unsigned tab_depth, std::unordered_set<const Value *> &visited) const;

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
    virtual void *get_data_pointer() {
        assert(false && "No override for raw data");
        return nullptr;
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
 
    ModuleValue *get_owner() {
        return this->owner;
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
    IntValue(opcode::IntConst value);
public:
    static const TypeKind ClassType = TypeKind::INT;

    #define BUILT_INS_INT_CONSTANTS_AM 262
    /// Returns array of interned Ints.
    /// @note This needs to be a function to set the order of initialization,
    ///       so that BuiltIns::Int is intialized first.
    static std::array<IntValue *, BUILT_INS_INT_CONSTANTS_AM> get_interned() {
        static std::array<IntValue *, BUILT_INS_INT_CONSTANTS_AM> IntConstants = {
            new IntValue(0), new IntValue(1), new IntValue(2), new IntValue(3),
            new IntValue(4), new IntValue(5), new IntValue(6), new IntValue(7),
            new IntValue(8), new IntValue(9), new IntValue(10), new IntValue(11),
            new IntValue(12), new IntValue(13), new IntValue(14), new IntValue(15),
            new IntValue(16), new IntValue(17), new IntValue(18), new IntValue(19),
            new IntValue(20), new IntValue(21), new IntValue(22), new IntValue(23),
            new IntValue(24), new IntValue(25), new IntValue(26), new IntValue(27),
            new IntValue(28), new IntValue(29), new IntValue(30), new IntValue(31),
            new IntValue(32), new IntValue(33), new IntValue(34), new IntValue(35),
            new IntValue(36), new IntValue(37), new IntValue(38), new IntValue(39),
            new IntValue(40), new IntValue(41), new IntValue(42), new IntValue(43),
            new IntValue(44), new IntValue(45), new IntValue(46), new IntValue(47),
            new IntValue(48), new IntValue(49), new IntValue(50), new IntValue(51),
            new IntValue(52), new IntValue(53), new IntValue(54), new IntValue(55),
            new IntValue(56), new IntValue(57), new IntValue(58), new IntValue(59),
            new IntValue(60), new IntValue(61), new IntValue(62), new IntValue(63),
            new IntValue(64), new IntValue(65), new IntValue(66), new IntValue(67),
            new IntValue(68), new IntValue(69), new IntValue(70), new IntValue(71),
            new IntValue(72), new IntValue(73), new IntValue(74), new IntValue(75),
            new IntValue(76), new IntValue(77), new IntValue(78), new IntValue(79),
            new IntValue(80), new IntValue(81), new IntValue(82), new IntValue(83),
            new IntValue(84), new IntValue(85), new IntValue(86), new IntValue(87),
            new IntValue(88), new IntValue(89), new IntValue(90), new IntValue(91),
            new IntValue(92), new IntValue(93), new IntValue(94), new IntValue(95),
            new IntValue(96), new IntValue(97), new IntValue(98), new IntValue(99),
            new IntValue(100), new IntValue(101), new IntValue(102), new IntValue(103),
            new IntValue(104), new IntValue(105), new IntValue(106), new IntValue(107),
            new IntValue(108), new IntValue(109), new IntValue(110), new IntValue(111),
            new IntValue(112), new IntValue(113), new IntValue(114), new IntValue(115),
            new IntValue(116), new IntValue(117), new IntValue(118), new IntValue(119),
            new IntValue(120), new IntValue(121), new IntValue(122), new IntValue(123),
            new IntValue(124), new IntValue(125), new IntValue(126), new IntValue(127),
            new IntValue(128), new IntValue(129), new IntValue(130), new IntValue(131),
            new IntValue(132), new IntValue(133), new IntValue(134), new IntValue(135),
            new IntValue(136), new IntValue(137), new IntValue(138), new IntValue(139),
            new IntValue(140), new IntValue(141), new IntValue(142), new IntValue(143),
            new IntValue(144), new IntValue(145), new IntValue(146), new IntValue(147),
            new IntValue(148), new IntValue(149), new IntValue(150), new IntValue(151),
            new IntValue(152), new IntValue(153), new IntValue(154), new IntValue(155),
            new IntValue(156), new IntValue(157), new IntValue(158), new IntValue(159),
            new IntValue(160), new IntValue(161), new IntValue(162), new IntValue(163),
            new IntValue(164), new IntValue(165), new IntValue(166), new IntValue(167),
            new IntValue(168), new IntValue(169), new IntValue(170), new IntValue(171),
            new IntValue(172), new IntValue(173), new IntValue(174), new IntValue(175),
            new IntValue(176), new IntValue(177), new IntValue(178), new IntValue(179),
            new IntValue(180), new IntValue(181), new IntValue(182), new IntValue(183),
            new IntValue(184), new IntValue(185), new IntValue(186), new IntValue(187),
            new IntValue(188), new IntValue(189), new IntValue(190), new IntValue(191),
            new IntValue(192), new IntValue(193), new IntValue(194), new IntValue(195),
            new IntValue(196), new IntValue(197), new IntValue(198), new IntValue(199),
            new IntValue(200), new IntValue(201), new IntValue(202), new IntValue(203),
            new IntValue(204), new IntValue(205), new IntValue(206), new IntValue(207),
            new IntValue(208), new IntValue(209), new IntValue(210), new IntValue(211),
            new IntValue(212), new IntValue(213), new IntValue(214), new IntValue(215),
            new IntValue(216), new IntValue(217), new IntValue(218), new IntValue(219),
            new IntValue(220), new IntValue(221), new IntValue(222), new IntValue(223),
            new IntValue(224), new IntValue(225), new IntValue(226), new IntValue(227),
            new IntValue(228), new IntValue(229), new IntValue(230), new IntValue(231),
            new IntValue(232), new IntValue(233), new IntValue(234), new IntValue(235),
            new IntValue(236), new IntValue(237), new IntValue(238), new IntValue(239),
            new IntValue(240), new IntValue(241), new IntValue(242), new IntValue(243),
            new IntValue(244), new IntValue(245), new IntValue(246), new IntValue(247),
            new IntValue(248), new IntValue(249), new IntValue(250), new IntValue(251),
            new IntValue(252), new IntValue(253), new IntValue(254), new IntValue(255),
            new IntValue(256),
            new IntValue(-1), new IntValue(-2), new IntValue(-3), new IntValue(-4),
            new IntValue(-5)
        };
        return IntConstants;
    }

    static IntValue *get(opcode::IntConst value) {
        if (value >= 0 && value <= 256)
            return get_interned()[value];
        else if (value < 0 && value >= -5)
            return get_interned()[256 - value]; // this will be -(-value) as it is negative
        return new IntValue(value);
    }

    virtual Value *clone() override {
        // Int is immutable (and also interned) and there is no need to copy it.
        return this;
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

    virtual void *get_data_pointer() override {
        return &value;
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

    FloatValue(opcode::FloatConst value);
public:
    static const TypeKind ClassType = TypeKind::FLOAT;

    static FloatValue *get(opcode::FloatConst value) {
        return new FloatValue(value);
    }
    
    virtual Value *clone() override {
        return this;
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

    virtual void *get_data_pointer() override {
        return &value;
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

    BoolValue(opcode::BoolConst value);
public:
    static const TypeKind ClassType = TypeKind::BOOL;

    BoolValue(const BoolValue&) = delete;            // no copying
    BoolValue& operator=(const BoolValue&) = delete; // no assignment

    static BoolValue* True() {
        static BoolValue instance(true);
        return &instance;
    }

    static BoolValue* False() {
        static BoolValue instance(false);
        return &instance;
    }

    static BoolValue *get(bool b) {
        if (b)
            return True();
        return False();
    }
    
    virtual Value *clone() override {
        return this;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return value ? 1L : 0L;
    }

    opcode::BoolConst get_value() { return this->value; }

    virtual opcode::StringConst as_string() const override {
        return value ? "true" : "false";
    }

    virtual void *get_data_pointer() override {
        return &value;
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Bool(" << (value ? "true" : "false") << ")";
        return os;
    }
};

/// Moss string value
class StringValue : public Value {
private:
    friend class StringIterator;
protected:
    opcode::StringConst value;
    
    StringValue(opcode::StringConst value);
public:
    static const TypeKind ClassType = TypeKind::STRING;

    static StringValue *get(opcode::StringConst value) {
        return new StringValue(value);
    }

    virtual Value *clone() override {
        // String is also immutable and so return it without copying;
        return this;
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

    virtual Value *iter(Interpreter *vm) override;

    virtual void *get_data_pointer() override {
        static const char * cstr_value = value.c_str();
        return &cstr_value;
    }

    virtual opcode::StringConst dump() override {
        return "\"" + utils::sanitize(value) + "\"";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "String(\"" << utils::sanitize(value) << "\")";
        return os;
    }
};

class StringIterator : public Value {
private:
    StringValue &value;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::STRING_ITER;

    StringIterator(StringValue &value);

    virtual Value *clone() override {
        return new StringIterator(value);
    }

    /// When mutable, then the value can change
    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; } // iter cannot be called on this.

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "StringIterator(" << value << ")";
        return os;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<StringIterator of String " << std::hex << static_cast<const void*>(&value) << ">";
        return ss.str();
    }

    virtual Value *iter(Interpreter *) override { return this; }
    virtual Value *next(Interpreter *vm) override;
};

/// Moss nil value (holds only one value)
class NilValue : public Value {
private:
    NilValue() : Value(ClassType, "Nil", BuiltIns::NilType) {}
public:
    static const TypeKind ClassType = TypeKind::NIL;

    static NilValue* Nil() {
        static NilValue instance;
        return &instance;
    }

    virtual Value *clone() override {
        return this;
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

class ListIterator;

class ListValue : public Value {
private:
    friend class ListIterator;
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

    std::vector<Value *> &get_vals() { return this->vals; }

    void push(Value *v) { vals.push_back(v); }
    size_t size() { return vals.size(); }
    void remove(long i) { vals.erase(vals.begin() + i); }
    void clear() {
        vals.clear();
    }

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

    virtual Value *iter(Interpreter *vm) override;

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

class ListIterator : public Value {
private:
    ListValue &value;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::LIST_ITER;

    ListIterator(ListValue &value);

    virtual Value *clone() override {
        return new ListIterator(value);
    }

    /// When mutable, then the value can change
    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; } // iter cannot be called on this.

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "ListIterator(" << value << ")";
        return os;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<ListIterator of List " << std::hex << static_cast<const void*>(&value) << ">";
        return ss.str();
    }

    virtual Value *iter(Interpreter *) override { return this; }
    virtual Value *next(Interpreter *vm) override;
};

class DictIterator;

class DictValue : public Value {
private:
    friend class DictIterator;
    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> vals;
public:
    static const TypeKind ClassType = TypeKind::DICT;

    // Since pushing a value might cause an exception there cannot be a constructor which takes list as is bellow.
    DictValue(std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> vals);
    DictValue();

    ~DictValue() {}

    virtual Value *clone() override {
        return new DictValue(this->vals);
    }

    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; }

    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> &get_vals() { return this->vals; }
    std::vector<std::pair<Value *, Value *>> vals_as_list();

    void push(Value *k, Value *v, Interpreter *vm);
    void push(std::vector<Value *> &keys, std::vector<Value *> &values, Interpreter *vm);
    void push(ListValue *keys, ListValue *values, Interpreter *vm) {
        push(keys->get_vals(), values->get_vals(), vm);
    }
    size_t size() {
        size_t s = 0;
        for (auto [k, v]: vals) {
            s += v.size();
        }
        return s;
    }
    void clear() {
        vals.clear();
    }

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

    virtual Value *iter(Interpreter *vm) override;

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

class DictIterator : public Value {
private:
    DictValue &value;
    std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>>::iterator iterator;
    size_t keys_iterator;
public:
    static const TypeKind ClassType = TypeKind::DICT_ITER;

    DictIterator(DictValue &value);

    virtual Value *clone() override {
        return new DictIterator(value);
    }

    /// When mutable, then the value can change
    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; } // iter cannot be called on this.

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "DictIterator(" << value << ")";
        return os;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<DictIterator of Dict " << std::hex << static_cast<const void*>(&value) << ">";
        return ss.str();
    }

    virtual Value *iter(Interpreter *) override { return this; }
    virtual Value *next(Interpreter *vm) override;
};

class BytesIterator;

/// Moss bytes value
class BytesValue : public Value {
private:
    friend class BytesIterator;
protected:
    const std::vector<uint8_t> value;
public:
    static const TypeKind ClassType = TypeKind::BYTES;

    BytesValue(std::vector<uint8_t> value);

    virtual Value *clone() override {
        // Bytes is immutable and so return it without copying;
        return this;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}(ustring(value.begin(), value.end()));
    }
    virtual inline bool is_iterable() override { return true; }

    std::vector<uint8_t> get_value() { return this->value; }

    virtual opcode::StringConst as_string() const override;

    virtual Value *iter(Interpreter *vm) override;

    virtual opcode::StringConst dump() override {
        return "b\"" + as_string() + "\"";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "Bytes(\"" << as_string() << "\")";
        return os;
    }
};

class BytesIterator : public Value {
private:
    BytesValue &value;
    size_t iterator;
public:
    static const TypeKind ClassType = TypeKind::BYTES_ITER;

    BytesIterator(BytesValue &value);

    virtual Value *clone() override {
        return new BytesIterator(value);
    }

    /// When mutable, then the value can change
    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; } // iter cannot be called on this.

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "BytesIterator(" << value << ")";
        return os;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<BytesIterator of Bytes " << std::hex << static_cast<const void*>(&value) << ">";
        return ss.str();
    }

    virtual Value *iter(Interpreter *) override { return this; }
    virtual Value *next(Interpreter *vm) override;
};

class ClassValue : public Value {
private:
    std::list<ClassValue *> supers;
public:
    static const TypeKind ClassType = TypeKind::CLASS;

    // If this is called by Type then Type is nullptr, so set type to this
    ClassValue(ustring name, ModuleValue *owner=nullptr) : Value(ClassType, name, BuiltIns::Type ? BuiltIns::Type : this, nullptr, owner) {}
    ClassValue(ustring name, std::list<ClassValue *> supers, ModuleValue *owner=nullptr) 
        : Value(ClassType, name,  BuiltIns::Type ? BuiltIns::Type : this, nullptr, owner), supers(supers) {}
    ClassValue(ustring name, MemoryPool *frm, std::list<ClassValue *> supers, ModuleValue *owner=nullptr) 
        : Value(ClassType, name, (BuiltIns::Type ? BuiltIns::Type : this), frm, owner), supers(supers) {}
    ~ClassValue() {}

    virtual Value *clone() override {
        /*auto cpy = new ClassValue(this->name, this->supers);
        cpy->set_attrs(this->attrs);
        cpy->annotations = this->annotations;
        return cpy;*/
        return this;
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
        auto all = get_all_supers();
        return std::find(all.begin(), all.end(), c) != all.end();
    }

    std::list<ClassValue *> get_all_supers();

    virtual std::ostream& debug(std::ostream& os) const override;
    virtual std::ostream& debug(std::ostream& os, unsigned tab_depth, std::unordered_set<const Value *> &visited) const override;
};

class ObjectValue : public Value {
public:
    static const TypeKind ClassType = TypeKind::OBJECT;

    ObjectValue(ClassValue *cls) : Value(ClassType, "<object>", cls) {
        // Note that ObjectValue does not need to set owner since it is already
        // done by it's type (class) and that is being marked by GC already.
        this->copy_attrs(cls->get_attrs());
    }

    ~ObjectValue();

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
    virtual std::ostream& debug(std::ostream& os, unsigned tab_depth, std::unordered_set<const Value *> &visited) const override;
};

class SpaceValue : public Value {
private:
    Interpreter *owner_vm;
    std::list<ModuleValue *> extra_owners; // Since space can be extended its functions may reside in multiple modules.
    bool anonymous;
public:
    static const TypeKind ClassType = TypeKind::SPACE;

    SpaceValue(ustring name, Interpreter *owner_vm, bool anonymous=false, ModuleValue *owner=nullptr)
        : Value(ClassType, name, BuiltIns::Space, nullptr, owner), owner_vm(owner_vm), anonymous(anonymous) {}
    SpaceValue(ustring name, MemoryPool *frm, Interpreter *owner_vm, bool anonymous=false, ModuleValue *owner=nullptr)
        : Value(ClassType, name, BuiltIns::Space, frm, owner), owner_vm(owner_vm), anonymous(anonymous) {}
    ~SpaceValue() {}

    virtual Value *clone() override {
        return this;
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

    void push_extra_owner(ModuleValue *m) {
        this->extra_owners.push_back(m);
    }

    const std::list<ModuleValue *> &get_extra_owners() {
        return this->extra_owners;
    }

    virtual std::ostream& debug(std::ostream& os) const override;
    virtual std::ostream& debug(std::ostream& os, unsigned tab_depth, std::unordered_set<const Value *> &visited) const override;
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
    ~ModuleValue() {
        // vm cannot be deleted here as it migth be used in other values.
    }

    virtual Value *clone() override {
        return this;
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
        // Note is immutable just like string.
        return this;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<opcode::StringConst>{}(format+value);
    }
    virtual inline bool is_iterable() override { return true; }

    opcode::StringConst get_format() { return this->format; }

    virtual opcode::StringConst dump() override {
        return utils::sanitize(format) + utils::sanitize(dyn_cast<StringValue>(this)->dump());
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

struct ExceptionCatch;

class FunValue : public Value {
private:
    std::vector<FunValueArg *> args;
    std::list<MemoryPool *> closures;
    Interpreter *vm;
    opcode::Address body_addr;
    ClassValue *parent_class;
    std::list<ExceptionCatch> catches;
public:
    static const TypeKind ClassType = TypeKind::FUN;

    FunValue(opcode::StringConst name, opcode::StringConst arg_names, Interpreter *vm, ModuleValue *owner=nullptr)
            : Value(ClassType, name, BuiltIns::Function, nullptr, owner), 
              args(), closures(), vm(vm), body_addr(0), parent_class(nullptr) {
        auto names = utils::split_csv(arg_names, ',');
        for (auto n: names) {
            args.push_back(new FunValueArg(n, std::vector<Value *>{}));
        }
    }

    FunValue(opcode::StringConst name,
             std::vector<FunValueArg *> args,
             Interpreter *vm,
             opcode::Address body_addr,
             ModuleValue *owner=nullptr) 
            : Value(ClassType, name, BuiltIns::Function, nullptr, owner), args(args), vm(vm),
              body_addr(body_addr), parent_class(nullptr) {}

    ~FunValue();

    virtual Value *clone() override {
        return this;
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0f_"+name);
    }

    void push_catch(ExceptionCatch c);

    std::list<ExceptionCatch> &get_catches() {
        return catches;
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

    bool is_lambda() const {
        assert(!name.empty() && "Function without name");
        return std::isdigit(name[0]);
    }

    void set_parent_class(ClassValue *c) { this->parent_class = c; }
    ClassValue *get_parent_class() { return this->parent_class; }
    bool is_constructor() { 
        return this->parent_class != nullptr && parent_class->get_name() == name;
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
            if (a->vararg)
                os << "... ";
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
            if (a->default_value) {
                os << "=" << a->default_value->dump();
            }
        }
        return os.str();
    }

    ustring get_signature() const {
        std::stringstream os;
        if (this->is_lambda())
            os << "<anonymous>";
        else
            os << name;
        os << "(" << get_args_as_str() << ")";
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

class FunctionListIterator;

class FunValueList : public Value {
private:
    friend class FunctionListIterator;
    std::vector<FunValue *> funs;
public:
    static const TypeKind ClassType = TypeKind::FUN_LIST;

    FunValueList(FunValue *f) : Value(ClassType, "FunctionList", BuiltIns::FunctionList) {
        funs.push_back(f);
    }
    FunValueList(std::vector<FunValue *> funs) : Value(ClassType, "FunctionList", BuiltIns::FunctionList), funs(funs) {}
    
    virtual Value *clone() override {
        return this;
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

    virtual Value *iter(Interpreter *vm) override;

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

class FunctionListIterator : public Value {
private:
    FunValueList &value;
    std::vector<FunValue *>::iterator iterator;
public:
    static const TypeKind ClassType = TypeKind::FUN_LIST_ITER;

    FunctionListIterator(FunValueList &value);

    virtual Value *clone() override {
        return new FunctionListIterator(value);
    }

    /// When mutable, then the value can change
    virtual inline bool is_hashable() override { return false; }
    virtual inline bool is_iterable() override { return true; } // iter cannot be called on this.

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "FunctionListIterator(" << value << ")";
        return os;
    }

    virtual opcode::StringConst as_string() const override {
        std::stringstream ss;
        ss << "<FunctionListIterator of Function " << std::hex << static_cast<const void*>(&value) << ">";
        return ss.str();
    }

    virtual Value *iter(Interpreter *) override { return this; }
    virtual Value *next(Interpreter *vm) override;
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

    EnumTypeValue(ustring name) : Value(ClassType, name, BuiltIns::Enum) {}
    EnumTypeValue(ustring name, std::vector<EnumValue *> vals) 
        : Value(ClassType, name, BuiltIns::Enum), vals(vals) {}

    virtual Value *clone() override {
        return this;
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
    ClassValue *parent;
public:
    static const TypeKind ClassType = TypeKind::SUPER_VALUE;

    SuperValue(ObjectValue *instance, ClassValue *parent)
        : Value(ClassType, "super", BuiltIns::super), instance(instance), parent(parent) {}
    ~SuperValue() {}

    virtual Value *clone() override {
        return new SuperValue(this->instance, this->parent);
    }

    virtual inline bool is_hashable() override { return true; }
    virtual opcode::IntConst hash() override {
        return std::hash<ustring>{}("0sp_"+name);
    }

    ObjectValue *get_instance() { return this->instance; }
    ClassValue *get_parent() { return this->parent; }

    virtual Value *get_attr(ustring name, Interpreter *caller_vm) override;

    virtual opcode::StringConst as_string() const override {
        return "<super of " + instance->get_type()->get_name() + " in class " + parent->get_name() + ">";
    }

    virtual std::ostream& debug(std::ostream& os) const override {
        os << "super(" << *instance << ", " << *parent << ")";
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

namespace t_cpp {
    class CppValue;
}

template<>
bool isa<t_cpp::CppValue>(Value& t);

template<>
bool isa<t_cpp::CppValue>(Value* t);

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

template<>
t_cpp::CppValue *dyn_cast(Value* t);

}

#endif//VALUES_HPP_