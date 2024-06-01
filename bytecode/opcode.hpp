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

#define BC_OPCODE_SIZE 1  /// Size of opcode in bytes
using opcode_t = uint8_t;

#define BC_REGISTER_SIZE 4 /// How many bytes does register index take
using Register = uint32_t;

#define BC_STR_LEN_SIZE 4 /// How many bytes does the string size take
using strlen_t = uint32_t;
using StringVal = ustring;

#define BC_ADDR_SIZE 4    /// How many bytes does bytecode address take
using Address = uint32_t;

#define BC_INT_SIZE 8 /// How many bytes does int take
using IntConst = int64_t;


enum OpCodes : opcode_t {
    END = 0, // End of code

    LOAD,        //   %dst, "name"
    LOAD_ATTR,   //   %dst, %src, "name"
    LOAD_GLOBAL, //   %dst, "name"
    LOAD_NONLOC, //   %dst, "name"

    STORE_NAME, //        %dst, "name"
    ALIAS, //             %dst, "name"
    STORE, //             %dst, %src
    STORE_CONST, //       %dst, #val
    STORE_ADDR, //        %dst, addr
    STORE_ATTR, //        %src, %obj, "name"
    STORE_ADDR_ATTR, //   addr, %obj, "name"
    STORE_CONST_ATTR, //  #val, %obj, "name"

    STORE_INT_CONST, //   #dst, int
    STORE_FLOAT_CONST, // #dst, float
    STORE_STR_CONST, //   #dst, "string"

    JMP, //               addr
    JMP_IF_TRUE, //       %src, addr
    JMP_IF_FALSE, //      %src, addr
    CALL, //              %dst, addr
    RETURN, //            %val
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
    NOT, //       %dst, %src1
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
public:
    virtual ~OpCode() {}

    OpCodes get_type() { return this->op_type; }
    virtual inline std::ostream& debug(std::ostream& os) const {
        os << mnem;
        return os;
    }
    virtual void exec(Interpreter *vm) = 0;
};

inline std::ostream& operator<< (std::ostream& os, OpCode &op) {
    return op.debug(os);
}

class End : public OpCode {
public:
    static const OpCodes ClassType = OpCodes::END;

    End() : OpCode(ClassType, "END") {}

    void exec(Interpreter *vm) override;
};

class Load : public OpCode {
private:
    Register dst;
    StringVal name;
public:
    static const OpCodes ClassType = OpCodes::LOAD;

    Load(Register dst, StringVal name) : OpCode(ClassType, "LOAD"), dst(dst), name(name) {}
    void exec(Interpreter *vm) override;
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
        return os;
    }
};

class StoreName : public OpCode {
private:
    Register dst;
    StringVal name;
public:
    static const OpCodes ClassType = OpCodes::STORE_NAME;

    StoreName(Register dst, StringVal name) : OpCode(ClassType, "STORE_NAME"), dst(dst), name(name) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", \"" << name << "\"";
        return os;
    }
};

class StoreConst : public OpCode {
private:
    Register dst;
    Register csrc;
public:
    static const OpCodes ClassType = OpCodes::STORE_CONST;

    StoreConst(Register dst, Register csrc) : OpCode(ClassType, "STORE_CONST"), dst(dst), csrc(csrc) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t%" << dst << ", " << " #" << csrc;
        return os;
    }
};

class StoreIntConst : public OpCode {
private:
    Register dst;
    IntConst val;
public:
    static const OpCodes ClassType = OpCodes::STORE_INT_CONST;

    StoreIntConst(Register dst, IntConst val) : OpCode(ClassType, "STORE_INT_CONST"), dst(dst), val(val) {}
    
    void exec(Interpreter *vm) override;
    
    virtual inline std::ostream& debug(std::ostream& os) const override {
        os << mnem << "\t#" << dst << ", " << val;
        return os;
    }
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