/// 
/// \file opcode.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode opcodes
/// 

#ifndef _OPCODE_HPP_
#define _OPCODE_HPP_

#include "commons.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include "diagnostics.hpp"
#include <cstdint>

namespace moss {

class ModuleValue;
class Interpreter;
namespace opcode {
    class OpCode;
}
class ObjectValue;

// Helper functions
template<class T>
bool isa(opcode::OpCode& o);

template<class T>
bool isa(opcode::OpCode* o);

template<class T>
T *dyn_cast(opcode::OpCode* o);

namespace annots {
    inline constexpr char INTERNAL[] = "internal";
};

namespace known_names {
    inline constexpr char TO_STRING_METHOD[] = "__String";
}

namespace opcode {

/// Opcode names and their corresponding number
enum OpCodes : opcode_t {
    END = 0, // End of code

    LOAD,        //   %dst, "name"
    LOAD_ATTR,   //   %dst, %src, "name"
    LOAD_GLOBAL, //   %dst, "name"
    LOAD_NONLOC, //   %dst, "name"

    STORE, //             %dst, %src
    STORE_NAME, //        %dst, "name"
    STORE_CONST, //       %dst, #val
    STORE_ATTR, //        %src, %obj, "name"
    STORE_CONST_ATTR, //  #val, %obj, "name"
    STORE_GLOBAL, //      %src, "name"
    STORE_NONLOC, //      %src, "name"
    STORE_SUBSC, //       %src, %obj, %key
    STORE_CONST_SUBSC, // #val, %obj, %key
    STORE_SUBSC_CONST, // %src, %obj, #ckey
    STORE_C_SUBSC_C, //   #val, %obj, #ckey

    STORE_INT_CONST, //   #dst, int
    STORE_FLOAT_CONST, // #dst, float
    STORE_BOOL_CONST, // #dst, bool
    STORE_STRING_CONST, //   #dst, "string"
    STORE_NIL_CONST, //   #dst

    JMP, //               addr
    JMP_IF_TRUE, //       %src, addr
    JMP_IF_FALSE, //      %src, addr
    CALL, //              %dst, fun
    CALL_FORMATTER, //    %dst, fun
    PUSH_FRAME, //
    POP_FRAME,  //
    PUSH_CALL_FRAME, //
    POP_CALL_FRAME,  //
    RETURN, //            %src
    RETURN_CONST, //      #val
    PUSH_ARG, //          %val
    PUSH_CONST_ARG, //    #val
    PUSH_NAMED_ARG, //    %val, "name"
    PUSH_UNPACKED, //     %val
    CREATE_FUN, //        %fun, "name", "arg csv"
    FUN_BEGIN,  //        %fun
    SET_DEFAULT, //       %fun, int, %src
    SET_DEFAULT_CONST, // %fun, int, #src
    SET_TYPE, //          %fun, int, %type
    SET_VARARG, //        %fun, int

    IMPORT, //        %dst, "name"
    IMPORT_ALL, //    %src

    PUSH_PARENT, //   %class
    BUILD_CLASS, //   %src

    ANNOTATE, //      %dst, "name", %val
    ANNOTATE_MOD, //  "name", %val
    DOCUMENT, //      %dst, "txt"

    OUTPUT, //    %src

    CONCAT, //    %dst, %src1, %src2
    EXP, //       %dst, %src1, %src2
    ADD, //       %dst, %src1, %src2
    SUB, //       %dst, %src1, %src2
    DIV, //       %dst, %src1, %src2
    MUL, //       %dst, %src1, %src2
    MOD, //       %dst, %src1, %src2
    EQ, //        %dst, %src1, %src2
    NEQ, //       %dst, %src1, %src2
    BT, //        %dst, %src1, %src2
    LT, //        %dst, %src1, %src2
    BEQ, //       %dst, %src1, %src2
    LEQ, //       %dst, %src1, %src2
    IN, //        %dst, %src1, %src2
    AND, //       %dst, %src1, %src2
    OR, //        %dst, %src1, %src2
    XOR, //       %dst, %src1, %src2
    SUBSC, //     %dst, %src, %index

    CONCAT2, //   %dst, #val, %src2
    EXP2, //      %dst, #val, %src2
    ADD2, //      %dst, #val, %src2
    SUB2, //      %dst, #val, %src2
    DIV2, //      %dst, #val, %src2
    MUL2, //      %dst, #val, %src2
    MOD2, //      %dst, #val, %src2
    EQ2, //       %dst, #val, %src2
    NEQ2, //      %dst, #val, %src2
    BT2, //       %dst, #val, %src2
    LT2, //       %dst, #val, %src2
    BEQ2, //      %dst, #val, %src2
    LEQ2, //      %dst, #val, %src2
    IN2, //       %dst, #val, %src2
    AND2, //      %dst, #val, %src2
    OR2, //       %dst, #val, %src2
    XOR2, //      %dst, #val, %src2
    SUBSC2, //    %dst, #src, %index

    CONCAT3, //   %dst, %src1, #val
    EXP3, //      %dst, %src1, #val
    ADD3, //      %dst, %src1, #val
    SUB3, //      %dst, %src1, #val
    DIV3, //      %dst, %src1, #val
    MUL3, //      %dst, %src1, #val
    MOD3, //      %dst, %src1, #val
    EQ3, //       %dst, %src1, #val
    NEQ3, //      %dst, %src1, #val
    BT3, //       %dst, %src1, #val
    LT3, //       %dst, %src1, #val
    BEQ3, //      %dst, %src1, #val
    LEQ3, //      %dst, %src1, #val
    IN3, //       %dst, %src1, #val
    AND3, //      %dst, %src1, #val
    OR3, //       %dst, %src1, #val
    XOR3, //      %dst, %src1, #val
    SUBSC3, //    %dst, %src, #index

    NOT, //       %dst, %src1
    NEG, //       %dst, %src1

    ASSERT, //    %src, %msg

    RAISE, //         %src
    CATCH,       //   "exc_name", addr
    CATCH_TYPED, //   "exc_name", "type", addr
    POP_CATCH,   //

    LIST_PUSH, //         %dst, %val
    LIST_PUSH_CONST, //   %dst, #val
    BUILD_LIST, //        %dst

    BUILD_DICT, //        %dst, %keys, %vals
    BUILD_ENUM, //        %dst, %vals, %name
    BUILD_SPACE,  //      %dst, "name"

    CREATE_RANGE, //      %dst, %start, %step, %end
    CREATE_RANGE2, //     %dst, #start, %step, %end
    CREATE_RANGE3, //     %dst, %start, #step, %end
    CREATE_RANGE4, //     %dst, %start, %step, #end
    CREATE_RANGE5, //     %dst, #start, #step, %end
    CREATE_RANGE6, //     %dst, #start, %step, #end
    CREATE_RANGE7, //     %dst, %start, #step, #end
    CREATE_RANGE8, //     %dst, #start, #step, #end

    SWITCH, //    %src, %listvals, %listaddr, addr_def
    FOR, //       %i, %iterator, addr
    ITER, // %iterator, %collection

    OPCODES_AMOUNT
};

/// \brief Raises moss interpreter exception
/// \param exc Value to raise
void raise(Value *exc);

/// \brief Loads module from a ms or msb file
/// \param vm VM doing the loading
/// \param name Name of the module (file extention will be added)
/// \return Loaded module or raises an exception
ModuleValue *load_module(Interpreter *vm, ustring name);

/// \brief Converts value to its string representation
/// This conversion might use primitive value convertors or runtime call to __String
/// \note This might raise an exception the runtime call to __String
/// \param vm VM doing the conversion
/// \param v Value to convert
/// \return v as string
StringConst to_string(Interpreter *vm, Value *v);

/// \return true if t1 is same as t2 or t1 is subtype of t2 (child class)
bool is_type_eq_or_subtype(Value *t1, Value *t2);

bool eq(Value *s1, Value *s2, Interpreter *vm);

void output_generator_notes(Interpreter *vm);

FunValue *lookup_method(Interpreter *vm, Value *obj, ustring name, std::initializer_list<Value *> args, diags::DiagID &err);

opcode::IntConst hash_obj(ObjectValue *obj, Interpreter *vm);

/// Base Opcode class
class OpCode {
protected:
    OpCodes op_type;
    ustring mnem;

    OpCode(OpCodes op_type, ustring mnem) : op_type(op_type), mnem(mnem) {
        static_assert(OpCodes::OPCODES_AMOUNT <= 0xFF && "Opcodes cannot fit into 1 byte");
    }

    void check_load(Value *v, Interpreter *vm);
public:
    virtual ~OpCode() {}

    OpCodes get_type() { return this->op_type; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << mnem;
        return os;
    }
    std::string err_mgs(std::string msg, Interpreter *vm);
    virtual bool equals(OpCode *other) = 0;
    virtual void exec(Interpreter *vm) = 0;
};

/// Binary expression opcode
class BinExprOpCode: public OpCode {
public:
    Register dst;
    Register src1;
    Register src2;

protected:
    BinExprOpCode(OpCodes code, ustring mnem, Register dst, Register src1, Register src2) 
        : OpCode(code, mnem), dst(dst), src1(src1), src2(src2) {}

public:
    virtual ~BinExprOpCode() {}

    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src1 << ", %" << src2;
        return os;
    }

    bool equals(OpCode *other) override {
        if (other->get_type() != get_type()) return false;
        auto casted = dynamic_cast<BinExprOpCode *>(other);
        return casted->dst == dst && casted->src1 == src1 && casted->src2 == src2;
    }
};

inline std::ostream& operator<< (std::ostream& os, OpCode &op) {
    return op.debug(os);
}

inline std::ostream& operator<< (std::ostream& os, const OpCode &op) {
    return op.debug(os);
}

inline bool operator==(OpCode &a, OpCode &b) {
    return a.equals(&b);
}

class End : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::END;

    End() : OpCode(ClassType, "END") {}

    void exec(Interpreter *vm) override;
    bool equals(OpCode *other) override {
        return isa<End>(other);
    }
};

class Load : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::LOAD;

    Load(Register dst, StringConst name) : OpCode(ClassType, "LOAD"), dst(dst), name(name) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Load>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class LoadAttr : public OpCode {
public:
    Register dst;
    Register src;
    StringConst name;

    static const OpCodes ClassType = OpCodes::LOAD_ATTR;

    LoadAttr(Register dst, Register src, StringConst name) : OpCode(ClassType, "LOAD_ATTR"), dst(dst), src(src), name(name) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<LoadAttr>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->src == src && casted->name == name;
    }
};

class LoadGlobal : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::LOAD_GLOBAL;

    LoadGlobal(Register dst, StringConst name) : OpCode(ClassType, "LOAD_GLOBAL"), dst(dst), name(name) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<LoadGlobal>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class LoadNonLoc : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::LOAD_NONLOC;

    LoadNonLoc(Register dst, StringConst name) : OpCode(ClassType, "LOAD_NONLOC"), dst(dst), name(name) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<LoadNonLoc>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class Store : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::STORE;

    Store(Register dst, Register src) : OpCode(ClassType, "STORE"), dst(dst), src(src) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Store>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->src == src;
    }
};

class StoreName : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::STORE_NAME;

    StoreName(Register dst, StringConst name) : OpCode(ClassType, "STORE_NAME"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreName>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class StoreConst : public OpCode {
public:
    Register dst;
    Register csrc;

    static const OpCodes ClassType = OpCodes::STORE_CONST;

    StoreConst(Register dst, Register csrc) : OpCode(ClassType, "STORE_CONST"), dst(dst), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", " << " #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->csrc == csrc;
    }
};

class StoreAttr : public OpCode {
public:
    Register src;
    Register obj;
    StringConst name;

    static const OpCodes ClassType = OpCodes::STORE_ATTR;

    StoreAttr(Register src, Register obj, StringConst name) : OpCode(ClassType, "STORE_ATTR"), src(src), obj(obj), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", %" << obj << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreAttr>(other);
        if (!casted) return false;
        return casted->src == src && casted->obj == obj && casted->name == name;
    }
};

class StoreConstAttr : public OpCode {
public:
    Register csrc;
    Register obj;
    StringConst name;

    static const OpCodes ClassType = OpCodes::STORE_CONST_ATTR;

    StoreConstAttr(Register csrc, Register obj, StringConst name) : OpCode(ClassType, "STORE_CONST_ATTR"), csrc(csrc), obj(obj), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << csrc << ", %" << obj << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreConstAttr>(other);
        if (!casted) return false;
        return casted->csrc == csrc && casted->obj == obj && casted->name == name;
    }
};

class StoreGlobal : public OpCode {
public:
    Register src;
    StringConst name;

    static const OpCodes ClassType = OpCodes::STORE_GLOBAL;

    StoreGlobal(Register src, StringConst name)
        : OpCode(ClassType, "STORE_GLOBAL"), src(src), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreGlobal>(other);
        if (!casted) return false;
        return casted->src == src && casted->name == name;
    }
};

class StoreNonLoc : public OpCode {
public:
    Register src;
    StringConst name;

    static const OpCodes ClassType = OpCodes::STORE_NONLOC;

    StoreNonLoc(Register src, StringConst name)
        : OpCode(ClassType, "STORE_NONLOC"), src(src), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreNonLoc>(other);
        if (!casted) return false;
        return casted->src == src && casted->name == name;
    }
};

class StoreSubsc : public OpCode {
public:
    Register src;
    Register obj;
    Register key;

    static const OpCodes ClassType = OpCodes::STORE_SUBSC;

    StoreSubsc(Register src, Register obj, Register key) 
        : OpCode(ClassType, "STORE_SUBSC"), src(src), obj(obj), key(key) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", %" << obj << ", %" << key;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreSubsc>(other);
        if (!casted) return false;
        return casted->src == src && casted->obj == obj && casted->key == key;
    }
};

class StoreConstSubsc : public OpCode {
public:
    Register csrc;
    Register obj;
    Register key;

    static const OpCodes ClassType = OpCodes::STORE_CONST_SUBSC;

    StoreConstSubsc(Register csrc, Register obj, Register key) 
        : OpCode(ClassType, "STORE_CONST_SUBSC"), csrc(csrc), obj(obj), key(key) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << csrc << ", %" << obj << ", %" << key;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreConstSubsc>(other);
        if (!casted) return false;
        return casted->csrc == csrc && casted->obj == obj && casted->key == key;
    }
};

class StoreSubscConst : public OpCode {
public:
    Register src;
    Register obj;
    Register ckey;

    static const OpCodes ClassType = OpCodes::STORE_SUBSC_CONST;

    StoreSubscConst(Register src, Register obj, Register ckey) 
        : OpCode(ClassType, "STORE_SUBSC_CONST"), src(src), obj(obj), ckey(ckey) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", %" << obj << ", #" << ckey;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreSubscConst>(other);
        if (!casted) return false;
        return casted->src == src && casted->obj == obj && casted->ckey == ckey;
    }
};

class StoreConstSubscConst : public OpCode {
public:
    Register csrc;
    Register obj;
    Register ckey;

    static const OpCodes ClassType = OpCodes::STORE_C_SUBSC_C;

    StoreConstSubscConst(Register csrc, Register obj, Register ckey) 
        : OpCode(ClassType, "STORE_C_SUBSC_C"), csrc(csrc), obj(obj), ckey(ckey) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << csrc << ", %" << obj << ", #" << ckey;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreConstSubscConst>(other);
        if (!casted) return false;
        return casted->csrc == csrc && casted->obj == obj && casted->ckey == ckey;
    }
};

class StoreIntConst : public OpCode {
public:
    Register dst;
    IntConst val;

    static const OpCodes ClassType = OpCodes::STORE_INT_CONST;

    StoreIntConst(Register dst, IntConst val) : OpCode(ClassType, "STORE_INT_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << dst << ", " << val;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreIntConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val;
    }
};

class StoreFloatConst : public OpCode {
public:
    Register dst;
    FloatConst val;

    static const OpCodes ClassType = OpCodes::STORE_FLOAT_CONST;

    StoreFloatConst(Register dst, FloatConst val) : OpCode(ClassType, "STORE_FLOAT_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << dst << ", " << val;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreFloatConst>(other);
        if (!casted) return false;
        // Note: Float comparison, but this should be used in tests only
        return casted->dst == dst && casted->val == val;
    }
};

// This could be just StoreTrue
class StoreBoolConst : public OpCode {
public:
    Register dst;
    BoolConst val;

    static const OpCodes ClassType = OpCodes::STORE_BOOL_CONST;

    StoreBoolConst(Register dst, BoolConst val) : OpCode(ClassType, "STORE_BOOL_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << dst << ", " << (val ? "true" : "false");
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreBoolConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val;
    }
};

class StoreStringConst : public OpCode {
public:
    Register dst;
    StringConst val;

    static const OpCodes ClassType = OpCodes::STORE_STRING_CONST;

    StoreStringConst(Register dst, StringConst val) : OpCode(ClassType, "STORE_STRING_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << dst << ", \"" << utils::sanitize(val) << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreStringConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val;
    }
};

class StoreNilConst : public OpCode {
public:
    Register dst;

    static const OpCodes ClassType = OpCodes::STORE_NIL_CONST;

    StoreNilConst(Register dst) : OpCode(ClassType, "STORE_NIL_CONST"), dst(dst) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << dst;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreNilConst>(other);
        if (!casted) return false;
        return casted->dst == dst;
    }
};

class Jmp : public OpCode {
public:
    Address addr;
    enum class JMPState {
        SET,
        NOT_SET_BREAK,
        NOT_SET_CONTINUE
    } state;

    static const OpCodes ClassType = OpCodes::JMP;

    Jmp(Address addr, JMPState state=JMPState::SET) : OpCode(ClassType, "JMP"), addr(addr), state(state) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Jmp>(other);
        if (!casted) return false;
        return casted->addr == addr;
    }
};

class JmpIfTrue : public OpCode {
public:
    Register src;
    Address addr;

    static const OpCodes ClassType = OpCodes::JMP_IF_TRUE;

    JmpIfTrue(Register src, Address addr) : OpCode(ClassType, "JMP_IF_TRUE"), src(src), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<JmpIfTrue>(other);
        if (!casted) return false;
        return casted->addr == addr && casted->src == src;
    }
};

class JmpIfFalse : public OpCode {
public:
    Register src;
    Address addr;

    static const OpCodes ClassType = OpCodes::JMP_IF_FALSE;

    JmpIfFalse(Register src, Address addr) : OpCode(ClassType, "JMP_IF_FALSE"), src(src), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<JmpIfFalse>(other);
        if (!casted) return false;
        return casted->addr == addr && casted->src == src;
    }
};

class Call : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::CALL;

    Call(Register dst, Register src) : OpCode(ClassType, "CALL"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Call>(other);
        if (!casted) return false;
        return casted->src == src && casted->dst == dst;
    }
};

class CallFormatter : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::CALL_FORMATTER;

    CallFormatter(Register dst, StringConst name)
        : OpCode(ClassType, "CALL_FORMATTER"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CallFormatter>(other);
        if (!casted) return false;
        return casted->name == name && casted->dst == dst;
    }
};

class PushFrame : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::PUSH_FRAME;

    PushFrame() : OpCode(ClassType, "PUSH_FRAME") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        return isa<PushFrame>(other);
    }
};

class PopFrame : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::POP_FRAME;

    PopFrame() : OpCode(ClassType, "POP_FRAME") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        return isa<PopFrame>(other);
    }
};

class PushCallFrame : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::PUSH_CALL_FRAME;

    PushCallFrame() : OpCode(ClassType, "PUSH_CALL_FRAME") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        return isa<PushCallFrame>(other);
    }
};

class PopCallFrame : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::POP_CALL_FRAME;

    PopCallFrame() : OpCode(ClassType, "POP_CALL_FRAME") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        return isa<PopCallFrame>(other);
    }
};

class Return : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::RETURN;

    Return(Register src) : OpCode(ClassType, "RETURN"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Return>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class ReturnConst : public OpCode {
public:
    Register csrc;

    static const OpCodes ClassType = OpCodes::RETURN_CONST;

    ReturnConst(Register csrc) : OpCode(ClassType, "RETURN_CONST"), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ReturnConst>(other);
        if (!casted) return false;
        return casted->csrc == csrc;
    }
};

class PushArg : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::PUSH_ARG;

    PushArg(Register src) : OpCode(ClassType, "PUSH_ARG"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushArg>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class PushConstArg : public OpCode {
public:
    Register csrc;

    static const OpCodes ClassType = OpCodes::PUSH_CONST_ARG;

    PushConstArg(Register csrc) : OpCode(ClassType, "PUSH_CONST_ARG"), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushConstArg>(other);
        if (!casted) return false;
        return casted->csrc == csrc;
    }
};

class PushNamedArg : public OpCode {
public:
    Register src;
    StringConst name;

    static const OpCodes ClassType = OpCodes::PUSH_NAMED_ARG;

    PushNamedArg(Register src, StringConst name) : OpCode(ClassType, "PUSH_NAMED_ARG"), src(src), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushNamedArg>(other);
        if (!casted) return false;
        return casted->src == src && casted->name == name;
    }
};

class PushUnpacked : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::PUSH_UNPACKED;

    PushUnpacked(Register src) : OpCode(ClassType, "PUSH_UNPACKED"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushUnpacked>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class CreateFun : public OpCode {
public:
    Register fun;
    StringConst name;
    StringConst arg_names;

    static const OpCodes ClassType = OpCodes::CREATE_FUN;

    CreateFun(Register fun, StringConst name, StringConst arg_names) 
                : OpCode(ClassType, "CREATE_FUN"), fun(fun), name(name), arg_names(arg_names) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun << ", \"" << name << "\"" << ", \"" << arg_names << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateFun>(other);
        if (!casted) return false;
        return casted->fun == fun && casted->name == name && casted->arg_names == arg_names;
    }
};

class FunBegin : public OpCode {
public:
    Register fun;

    static const OpCodes ClassType = OpCodes::FUN_BEGIN;

    FunBegin(Register fun) : OpCode(ClassType, "FUN_BEGIN"), fun(fun) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<FunBegin>(other);
        if (!casted) return false;
        return casted->fun == fun;
    }
};

class SetDefault : public OpCode {
public:
    Register fun;
    IntConst index;
    Register src;

    static const OpCodes ClassType = OpCodes::SET_DEFAULT;

    SetDefault(Register fun, IntConst index, Register src) 
                : OpCode(ClassType, "SET_DEFAULT"), fun(fun), index(index), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun << ", " << index << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<SetDefault>(other);
        if (!casted) return false;
        return casted->fun == fun && casted->src == src && casted->index == index;
    }
};

class SetDefaultConst : public OpCode {
public:
    Register fun;
    IntConst index;
    Register csrc;

    static const OpCodes ClassType = OpCodes::SET_DEFAULT_CONST;

    SetDefaultConst(Register fun, IntConst index, Register csrc) 
                : OpCode(ClassType, "SET_DEFAULT_CONST"), fun(fun), index(index), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun << ", " << index << ", #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<SetDefaultConst>(other);
        if (!casted) return false;
        return casted->fun == fun && casted->csrc == csrc && casted->index == index;
    }
};

class SetType : public OpCode {
public:
    Register fun;
    IntConst index;
    Register type;

    static const OpCodes ClassType = OpCodes::SET_TYPE;

    SetType(Register fun, IntConst index, Register type) 
                : OpCode(ClassType, "SET_TYPE"), fun(fun), index(index), type(type) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun << ", " << index << ", %" << type << "";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<SetType>(other);
        if (!casted) return false;
        return casted->fun == fun && casted->type == type && casted->index == index;
    }
};

class SetVararg : public OpCode {
public:
    Register fun;
    IntConst index;

    static const OpCodes ClassType = OpCodes::SET_VARARG;

    SetVararg(Register fun, IntConst index) : OpCode(ClassType, "SET_VARARG"), fun(fun), index(index) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << fun << ", " << index;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<SetVararg>(other);
        if (!casted) return false;
        return casted->fun == fun && casted->index == index;
    }
};

class Import : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::IMPORT;

    Import(Register dst, StringConst name) : OpCode(ClassType, "IMPORT"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Import>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class ImportAll : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::IMPORT_ALL;

    ImportAll(Register src) : OpCode(ClassType, "IMPORT_ALL"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ImportAll>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class PushParent : public OpCode {
public:
    Register parent;

    static const OpCodes ClassType = OpCodes::PUSH_PARENT;

    PushParent(Register parent) : OpCode(ClassType, "PUSH_PARENT"), parent(parent) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << parent;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushParent>(other);
        if (!casted) return false;
        return casted->parent == parent;
    }
};


class BuildClass : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::BUILD_CLASS;

    BuildClass(Register dst, StringConst name) : OpCode(ClassType, "BUILD_CLASS"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildClass>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class Annotate : public OpCode {
public:
    Register dst;
    StringConst name;
    Register val;

    static const OpCodes ClassType = OpCodes::ANNOTATE;

    Annotate(Register dst, StringConst name, Register val) : OpCode(ClassType, "ANNOTATE"), dst(dst), name(name), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\", %" << val;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Annotate>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val && casted->name == name;
    }
};

class AnnotateMod : public OpCode {
public:
    StringConst name;
    Register val;

    static const OpCodes ClassType = OpCodes::ANNOTATE_MOD;

    AnnotateMod(StringConst name, Register val) : OpCode(ClassType, "ANNOTATE_MOD"), name(name), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\"" << name << "\", %" << val;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<AnnotateMod>(other);
        if (!casted) return false;
        return casted->val == val && casted->name == name;
    }
};

class Document : public OpCode {
public:
    Register dst;
    StringConst val;

    static const OpCodes ClassType = OpCodes::DOCUMENT;

    Document(Register dst, StringConst val) : OpCode(ClassType, "DOCUMENT"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << utils::sanitize(val) << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Document>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val;
    }
};

class Output : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::OUTPUT;

    Output(Register src) : OpCode(ClassType, "OUTPUT"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Output>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class Concat : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::CONCAT;
    Concat(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "CONCAT", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Concat2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::CONCAT2;
    Concat2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "CONCAT2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Concat3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::CONCAT3;
    Concat3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "CONCAT3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Exp : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EXP;
    Exp(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "EXP", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Exp2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EXP2;
    Exp2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "EXP2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Exp3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EXP3;
    Exp3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "EXP3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Add : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::ADD;
    Add(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "ADD", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Add2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::ADD2;
    Add2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "ADD2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Add3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::ADD3;
    Add3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "ADD3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Sub : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUB;
    Sub(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "SUB", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Sub2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUB2;
    Sub2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "SUB2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Sub3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUB3;
    Sub3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "SUB3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Div : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::DIV;
    Div(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "DIV", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Div2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::DIV2;
    Div2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "DIV2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Div3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::DIV3;
    Div3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "DIV3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Mul : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MUL;
    Mul(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "MUL", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Mul2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MUL2;
    Mul2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "MUL2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Mul3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MUL3;
    Mul3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "MUL3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Mod : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MOD;
    Mod(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "MOD", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Mod2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MOD2;
    Mod2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "MOD2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Mod3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::MOD3;
    Mod3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "MOD3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Eq : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EQ;
    Eq(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "EQ", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Eq2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EQ2;
    Eq2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "EQ2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Eq3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::EQ3;
    Eq3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "EQ3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Neq : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::NEQ;
    Neq(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "NEQ", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Neq2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::NEQ2;
    Neq2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "NEQ2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Neq3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::NEQ3;
    Neq3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "NEQ3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Bt : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BT;
    Bt(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "BT", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Bt2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BT2;
    Bt2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "BT2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Bt3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BT3;
    Bt3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "BT3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Lt : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LT;
    Lt(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "LT", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Lt2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LT2;
    Lt2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "LT2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Lt3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LT3;
    Lt3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "LT3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Beq : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BEQ;
    Beq(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "BEQ", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Beq2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BEQ2;
    Beq2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "BEQ2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Beq3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::BEQ3;
    Beq3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "BEQ3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Leq : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LEQ;
    Leq(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "LEQ", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Leq2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LEQ2;
    Leq2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "LEQ2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Leq3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::LEQ3;
    Leq3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "LEQ3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class In : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::IN;
    In(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "IN", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class In2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::IN2;
    In2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "IN2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class In3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::IN3;
    In3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "IN3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class And : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::AND;
    And(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "AND", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class And2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::AND2;
    And2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "AND2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class And3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::AND3;
    And3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "AND3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Or : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::OR;
    Or(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "OR", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Or2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::OR2;
    Or2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "OR2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Or3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::OR3;
    Or3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "OR3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Xor : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::XOR;
    Xor(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "XOR", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Xor2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::XOR2;
    Xor2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "XOR2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Xor3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::XOR3;
    Xor3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "XOR3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Subsc : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUBSC;
    Subsc(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "SUBSC", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};

class Subsc2 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUBSC2;
    Subsc2(Register dst, Register csrc1, Register src2) : BinExprOpCode(ClassType, "SUBSC2", dst, csrc1, src2) {}
    void exec(Interpreter *vm) override;
};

class Subsc3 : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::SUBSC3;
    Subsc3(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "SUBSC3", dst, src1, csrc2) {}
    void exec(Interpreter *vm) override;
};

class Not : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::NOT;

    Not(Register dst, Register src) : OpCode(ClassType, "NOT"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Not>(other);
        if (!casted) return false;
        return casted->src == src && casted->dst == dst;
    }
};

class Neg : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::NEG;

    Neg(Register dst, Register src) : OpCode(ClassType, "NEG"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Neg>(other);
        if (!casted) return false;
        return casted->src == src && casted->dst == dst;
    }
};

class Assert : public OpCode {
public:
    Register src;
    Register msg;

    static const OpCodes ClassType = OpCodes::ASSERT;

    Assert(Register src, Register msg) : OpCode(ClassType, "ASSERT"), src(src), msg(msg) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", %" << msg;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Assert>(other);
        if (!casted) return false;
        return casted->src == src && casted->msg == msg;
    }
};

class Raise : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::RAISE;

    Raise(Register src) : OpCode(ClassType, "RAISE"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Raise>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class Catch : public OpCode {
public:
    StringConst name;
    Address addr;

    static const OpCodes ClassType = OpCodes::CATCH;

    Catch(StringConst name, Address addr) : OpCode(ClassType, "CATCH"), name(name), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  \"" << name << "\", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Catch>(other);
        if (!casted) return false;
        return casted->name == name && casted->addr == addr;
    }
};

class CatchTyped : public OpCode {
public:
    StringConst name;
    Register type;
    Address addr;

    static const OpCodes ClassType = OpCodes::CATCH_TYPED;

    CatchTyped(StringConst name, Register type, Address addr)
        : OpCode(ClassType, "CATCH_TYPED"), name(name), type(type), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  \"" << name << "\", %" << type << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CatchTyped>(other);
        if (!casted) return false;
        return casted->name == name && casted->addr == addr && casted->type == type;
    }
};

class PopCatch : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::POP_CATCH;

    PopCatch() : OpCode(ClassType, "POP_CATCH") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PopCatch>(other);
        return casted != nullptr;
    }
};

class ListPush : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::LIST_PUSH;

    ListPush(Register dst, Register src) : OpCode(ClassType, "LIST_PUSH"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ListPush>(other);
        if (!casted) return false;
        return casted->src == src && casted->dst == dst;
    }
};

class ListPushConst : public OpCode {
public:
    Register dst;
    Register csrc;

    static const OpCodes ClassType = OpCodes::LIST_PUSH_CONST;

    ListPushConst(Register dst, Register csrc) : OpCode(ClassType, "LIST_PUSH_CONST"), dst(dst), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ListPushConst>(other);
        if (!casted) return false;
        return casted->csrc == csrc && casted->dst == dst;
    }
};

class BuildList : public OpCode {
public:
    Register dst;

    static const OpCodes ClassType = OpCodes::BUILD_LIST;

    BuildList(Register dst) : OpCode(ClassType, "BUILD_LIST"), dst(dst) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildList>(other);
        if (!casted) return false;
        return casted->dst == dst;
    }
};

class BuildDict : public OpCode {
public:
    Register dst;
    Register keys;
    Register vals;

    static const OpCodes ClassType = OpCodes::BUILD_DICT;

    BuildDict(Register dst, Register keys, Register vals) 
              : OpCode(ClassType, "BUILD_DICT"), dst(dst), keys(keys), vals(vals) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << keys << ", %" << vals;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildDict>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->keys == keys && casted->vals == vals;
    }
};

class BuildEnum : public OpCode {
public:
    Register dst;
    Register vals;
    StringConst name;

    static const OpCodes ClassType = OpCodes::BUILD_ENUM;

    BuildEnum(Register dst, Register vals, StringConst name)
        : OpCode(ClassType, "BUILD_ENUM"), dst(dst), vals(vals), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << vals << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildEnum>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name && casted->vals == vals;
    }
};

class BuildSpace : public OpCode {
public:
    Register dst;
    StringConst name;
    bool anonymous;

    static const OpCodes ClassType = OpCodes::BUILD_SPACE;

    BuildSpace(Register dst, StringConst name, bool anonymous=false)
        : OpCode(ClassType, "BUILD_SPACE"), dst(dst), name(name), anonymous(anonymous) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildSpace>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class CreateRange : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE;

    CreateRange(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << start << ", %" << step << ", %" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange2 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE2;

    CreateRange2(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE2"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", #" << start << ", %" << step << ", %" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange2>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange3 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE3;

    CreateRange3(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE3"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << start << ", #" << step << ", %" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange3>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange4 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE4;

    CreateRange4(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE4"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << start << ", %" << step << ", #" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange4>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange5 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE5;

    CreateRange5(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE5"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", #" << start << ", #" << step << ", %" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange5>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange6 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE6;

    CreateRange6(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE6"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", #" << start << ", %" << step << ", #" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange6>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange7 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE7;

    CreateRange7(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE7"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", %" << start << ", #" << step << ", #" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange7>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class CreateRange8 : public OpCode {
public:
    Register dst;
    Register start;
    Register step;
    Register end;

    static const OpCodes ClassType = OpCodes::CREATE_RANGE8;

    CreateRange8(Register dst, Register start, Register step, Register end) :
        OpCode(ClassType, "CREATE_RANGE8"), dst(dst), start(start), step(step), end(end) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << dst << ", #" << start << ", #" << step << ", #" << end;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateRange8>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->start == start && casted->step == step && casted->end == end;
    }
};

class Switch : public OpCode {
public:
    Register src;
    Register vals;
    Register addrs;
    Register default_addr;

    static const OpCodes ClassType = OpCodes::SWITCH;

    Switch(Register src, Register vals, Register addrs, Register default_addr) :
        OpCode(ClassType, "SWITCH"), src(src), vals(vals), addrs(addrs), default_addr(default_addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << src << ", %" << vals << ", %" << addrs << ", %" << default_addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Switch>(other);
        if (!casted) return false;
        return casted->src == src && casted->vals == vals 
            && casted->addrs == addrs && casted->default_addr == default_addr;
    }
};

class For : public OpCode {
public:
    Register index;
    Register collection;
    Address  addr;

    static const OpCodes ClassType = OpCodes::FOR;

    For(Register index, Register collection, Address addr)
        : OpCode(ClassType, "FOR"), index(index), collection(collection), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "  %" << index << ", %" << collection << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<For>(other);
        if (!casted) return false;
        return casted->index == index && casted->collection == collection && casted->addr == addr;
    }
};

class Iter : public OpCode {
public:
    Register iterator;
    Register collection;

    static const OpCodes ClassType = OpCodes::ITER;

    Iter(Register iterator, Register collection) : OpCode(ClassType, "ITER"), iterator(iterator), collection(collection) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << " %" << iterator << ", %" << collection;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Iter>(other);
        if (!casted) return false;
        return casted->collection == collection && casted->iterator == iterator;
    }
};

}

// Helper functions
template<class T>
bool isa(opcode::OpCode& o) {
    return o.get_type() == T::ClassType;
}

template<class T>
bool isa(opcode::OpCode* o) {
    return o->get_type() == T::ClassType;
}

template<class T>
T *dyn_cast(opcode::OpCode* o) {
    if (!isa<T>(o)) return nullptr;
    return dynamic_cast<T *>(o);
}

}

#endif//_OPCODE_HPP_