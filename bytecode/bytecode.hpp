/**
 * @file bytecode.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief VM bytecode
 */

#ifndef _BYTECODE_HPP_
#define _BYTECODE_HPP_

#include <cstdint>

namespace moss {

class Bytecode {
public:
    enum OpCodes : uint8_t {
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

};

}

#endif//_BYTECODE_HPP_