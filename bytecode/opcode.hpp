/**
 * @file opcode.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Bytecode opcodes
 */

#ifndef _OPCODE_HPP_
#define _OPCODE_HPP_

#include "os_interface.hpp"
#include "interpreter.hpp"
#include <cstdint>

namespace moss {

class Interpreter;
namespace opcode {
    class OpCode;
}

// Helper functions
template<class T>
bool isa(opcode::OpCode& o);

template<class T>
bool isa(opcode::OpCode* o);

template<class T>
T *dyn_cast(opcode::OpCode* o);

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
    STORE_ADDR, //        %dst, addr
    STORE_ATTR, //        %src, %obj, "name"
    STORE_ADDR_ATTR, //   addr, %obj, "name"
    STORE_CONST_ATTR, //  #val, %obj, "name"

    STORE_INT_CONST, //   #dst, int
    STORE_FLOAT_CONST, // #dst, float
    STORE_BOOL_CONST, // #dst, bool
    STORE_STR_CONST, //   #dst, "string"
    STORE_NIL_CONST, //   #dst

    JMP, //               addr
    JMP_IF_TRUE, //       %src, addr
    JMP_IF_FALSE, //      %src, addr
    CALL, //              %dst, addr
    RETURN, //            %src
    RETURN_CONST, //      #val
    RETURN_ADDR, //       addr
    PUSH_ARG, //          %val
    PUSH_CONST_ARG, //    #val
    PUSH_ADDR_ARG, //     addr

    IMPORT, //        %dst, "name"
    IMPORT_ALL, //    "name"

    PUSH_PARENT, //   %class
    CREATE_OBJ, //    %dst, %class
    PROMOTE_OBJ, //   %src, %class
    BUILD_CLASS, //   %src
    COPY, //          %dst, %src
    DEEP_COPY, //     %dst, %src

    CREATE_ANNT, //   %dst, "name"
    ANNOTATE, //      %dst, %annot

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
    SC_AND, //    %dst, %src1, %src2
    SC_OR, //     %dst, %src1, %src2
    SUBSC, //     %dst, %src, %index
    SLICE, //     %dst, %src, %range

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
    SC_AND2, //   %dst, #val, %src2
    SC_OR2, //    %dst, #val, %src2
    SUBSC2, //    %dst, #src, %index
    SLICE2, //    %dst, #src, %range

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
    SC_AND3, //   %dst, %src1, #val
    SC_OR3, //    %dst, %src1, #val
    SUBSC3, //    %dst, %src, #index

    NOT, //       %dst, %src1

    ASSERT, //    %src

    COPY_ARGS, //

    RAISE, //         %val
    CHECK_CATCH, //   %dst, %class

    LIST_PUSH, //         %val
    LIST_PUSH_CONST, //   #val
    LIST_PUSH_ADDR, //    addr
    BUILD_LIST, //        %dst

    BUILD_DICT, //        %keys, %vals

    CREATE_RANGE, //      %dst, %start, %step, %end
    CREATE_RANGE2, //     %dst, #start, %step, %end
    CREATE_RANGE3, //     %dst, %start, #step, %end
    CREATE_RANGE4, //     %dst, %start, %step, #end
    CREATE_RANGE5, //     %dst, #start, #step, %end
    CREATE_RANGE6, //     %dst, #start, %step, #end
    CREATE_RANGE7, //     %dst, %start, #step, #end
    CREATE_RANGE8, //     %dst, #start, #step, #end

    SWITCH, //    %listvals, %listaddr, addr_def
    FOR, //       %i, %iterator

    BYTE_CODES_AMOUNT
};  //static_assert(Bytecode::BYTE_CODES_AMOUNT <= 0xFF && "Opcodes cannot fit into 1 byte");

/** Base Opcode class */
class OpCode {
protected:
    OpCodes op_type;
    ustring mnem;

    OpCode(OpCodes op_type, ustring mnem) : op_type(op_type), mnem(mnem) {}

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
        os << mnem << "\t%" << dst << ", %" << src1 << ", %" << src2;
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
        os << mnem << "\t\t%" << dst << ", \"" << name << "\"";
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
        os << mnem << "\t%" << dst << ", %" << src << ", \"" << name << "\"";
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
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
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
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
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
        os << mnem << "\t%" << dst << ", %" << src;
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
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
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
        os << mnem << "\t%" << dst << ", " << " #" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->csrc == csrc;
    }
};

class StoreAddr : public OpCode {
public:
    Register dst;
    Address addr;

    static const OpCodes ClassType = OpCodes::STORE_ADDR;

    StoreAddr(Register dst, Address addr) : OpCode(ClassType, "STORE_ADDR"), dst(dst), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreAddr>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->addr == addr;
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
        os << mnem << "\t%" << src << ", %" << obj << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreAttr>(other);
        if (!casted) return false;
        return casted->src == src && casted->obj == obj && casted->name == name;
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
        os << mnem << "\t#" << dst << ", " << val;
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
        os << mnem << "\t#" << dst << ", " << val;
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
        os << mnem << "\t#" << dst << ", " << (val ? "true" : "false");
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreBoolConst>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->val == val;
    }
};

class StoreStrConst : public OpCode {
public:
    Register dst;
    StringConst val;

    static const OpCodes ClassType = OpCodes::STORE_STR_CONST;

    StoreStrConst(Register dst, StringConst val) : OpCode(ClassType, "STORE_STR_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t#" << dst << ", \"" << val << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<StoreStrConst>(other);
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
        os << mnem << "\t#" << dst;
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

    static const OpCodes ClassType = OpCodes::JMP;

    Jmp(Address addr) : OpCode(ClassType, "JMP"), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t" << addr;
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
        os << mnem << "\t%" << src << ", " << addr;
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
        os << mnem << "\t%" << src << ", " << addr;
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
    Address addr;

    static const OpCodes ClassType = OpCodes::CALL;

    Call(Register dst, Address addr) : OpCode(ClassType, "CALL"), dst(dst), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", " << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Call>(other);
        if (!casted) return false;
        return casted->addr == addr && casted->dst == dst;
    }
};

class Return : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::RETURN;

    Return(Register src) : OpCode(ClassType, "RETURN"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << src;
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
        os << mnem << "\t%" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ReturnConst>(other);
        if (!casted) return false;
        return casted->csrc == csrc;
    }
};

class ReturnAddr : public OpCode {
public:
    Address addr;

    static const OpCodes ClassType = OpCodes::RETURN_ADDR;

    ReturnAddr(Address addr) : OpCode(ClassType, "RETURN_ADDR"), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t" << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ReturnAddr>(other);
        if (!casted) return false;
        return casted->addr == addr;
    }
};

class PushArg : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::PUSH_ARG;

    PushArg(Register src) : OpCode(ClassType, "PUSH_ARG"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << src;
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
        os << mnem << "\t%" << csrc;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushConstArg>(other);
        if (!casted) return false;
        return casted->csrc == csrc;
    }
};

class PushAddrArg : public OpCode {
public:
    Address addr;

    static const OpCodes ClassType = OpCodes::PUSH_ADDR_ARG;

    PushAddrArg(Address addr) : OpCode(ClassType, "PUSH_ADDR_ARG"), addr(addr) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t" << addr;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushAddrArg>(other);
        if (!casted) return false;
        return casted->addr == addr;
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
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
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
    StringConst name;

    static const OpCodes ClassType = OpCodes::IMPORT_ALL;

    ImportAll(StringConst name) : OpCode(ClassType, "IMPORT_ALL"), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t\"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<ImportAll>(other);
        if (!casted) return false;
        return casted->name == name;
    }
};

class PushParent : public OpCode {
public:
    Register parent;

    static const OpCodes ClassType = OpCodes::PUSH_PARENT;

    PushParent(Register parent) : OpCode(ClassType, "PUSH_PARENT"), parent(parent) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << parent;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PushParent>(other);
        if (!casted) return false;
        return casted->parent == parent;
    }
};

class CreateObject : public OpCode {
public:
    Register dst;
    Register cls;

    static const OpCodes ClassType = OpCodes::CREATE_OBJ;

    CreateObject(Register dst, Register cls) : OpCode(ClassType, "CREATE_OBJ"), dst(dst), cls(cls) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", %" << cls;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateObject>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->cls == cls;
    }
};

class PromoteObject : public OpCode {
public:
    Register src;
    Register cls;

    static const OpCodes ClassType = OpCodes::PROMOTE_OBJ;

    PromoteObject(Register src, Register cls) : OpCode(ClassType, "PROMOTE_OBJ"), src(src), cls(cls) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << src << ", %" << cls;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<PromoteObject>(other);
        if (!casted) return false;
        return casted->src == src && casted->cls == cls;
    }
};

class BuildClass : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::BUILD_CLASS;

    BuildClass(Register src) : OpCode(ClassType, "BUILD_CLASS"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<BuildClass>(other);
        if (!casted) return false;
        return casted->src == src;
    }
};

class Copy : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::COPY;

    Copy(Register dst, Register src) : OpCode(ClassType, "COPY"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Copy>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->src == src;
    }
};

class DeepCopy : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::DEEP_COPY;

    DeepCopy(Register dst, Register src) : OpCode(ClassType, "DEEP_COPY"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<DeepCopy>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->src == src;
    }
};

class CreateAnnt : public OpCode {
public:
    Register dst;
    StringConst name;

    static const OpCodes ClassType = OpCodes::CREATE_ANNT;

    CreateAnnt(Register dst, StringConst name) : OpCode(ClassType, "CREATE_ANNT"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<CreateAnnt>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->name == name;
    }
};

class Annotate : public OpCode {
public:
    Register dst;
    Register src;

    static const OpCodes ClassType = OpCodes::ANNOTATE;

    Annotate(Register dst, Register src) : OpCode(ClassType, "ANNOTATE"), dst(dst), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", %" << src;
        return os;
    }
    bool equals(OpCode *other) override {
        auto casted = dyn_cast<Annotate>(other);
        if (!casted) return false;
        return casted->dst == dst && casted->src == src;
    }
};

class Output : public OpCode {
public:
    Register src;

    static const OpCodes ClassType = OpCodes::OUTPUT;

    Output(Register src) : OpCode(ClassType, "OUTPUT"), src(src) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << src;
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
    Exp(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "EXP", dst, src1, csrc2) {}
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
    Add(Register dst, Register src1, Register csrc2) : BinExprOpCode(ClassType, "ADD", dst, src1, csrc2) {}
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

/*
class Name : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::NAME;

    Name() : OpCode(ClassType, "NAME") {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem;
        return os;
    }
    bool equals(OpCode *other) override {
        assert(false && "TODO: equals")
    }
};

class Name : public BinExprOpCode {
public:
    static const OpCodes ClassType = OpCodes::NAME;
    Name(Register dst, Register src1, Register src2) : BinExprOpCode(ClassType, "NAME", dst, src1, src2) {}
    void exec(Interpreter *vm) override;
};
*/

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