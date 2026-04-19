#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <cmath>
#include "bytecode.hpp"
#include "opcode.hpp"
#include "values.hpp"
#include "testing_utils.hpp"

namespace{

using namespace moss;
using namespace opcode;
using namespace testing;

TEST(BytecodeTransforms, Inserting) {
    Bytecode *bc = new Bytecode();

    bc->push_back(new opcode::Jmp(7));
    bc->push_back(new opcode::Jmp(3));
    bc->push_back(new opcode::StoreIntConst(300, 7));
    bc->push_back(new opcode::StoreIntConst(301, 42));
    // < Insert
    bc->push_back(new opcode::Jmp(5));
    bc->push_back(new opcode::StoreConst(0, 301));
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::End());

    bc->insert(new StoreFloatConst(323, 0.5), 4);

    std::stringstream ss;
    ss << *bc;

    ustring expected = 
"0\tJMP  8\n"
"1\tJMP  3\n"
"2\tSTORE_INT_CONST  #300, 7\n"
"3\tSTORE_INT_CONST  #301, 42\n"
"4\tSTORE_FLOAT_CONST  #323, 0.5\n"
"5\tJMP  6\n"
"6\tSTORE_CONST  %0,  #301\n"
"7\tSTORE_CONST  %0,  #300\n"
"8\tEND\n";

    EXPECT_EQ(ss.str(), expected);

    bc->insert(new StoreBoolConst(322, true), 9);
    std::stringstream ss2;
    ss2 << *bc;

    ustring expected2 = 
"0\tJMP  8\n"
"1\tJMP  3\n"
"2\tSTORE_INT_CONST  #300, 7\n"
"3\tSTORE_INT_CONST  #301, 42\n"
"4\tSTORE_FLOAT_CONST  #323, 0.5\n"
"5\tJMP  6\n"
"6\tSTORE_CONST  %0,  #301\n"
"7\tSTORE_CONST  %0,  #300\n"
"8\tEND\n"
"9\tSTORE_BOOL_CONST  #322, true\n";

    EXPECT_EQ(ss2.str(), expected2);

    bc->insert(new Jmp(10), 0);
    std::stringstream ss3;
    ss3 << *bc;

    ustring expected3 =
"0\tJMP  10\n" 
"1\tJMP  9\n"
"2\tJMP  4\n"
"3\tSTORE_INT_CONST  #300, 7\n"
"4\tSTORE_INT_CONST  #301, 42\n"
"5\tSTORE_FLOAT_CONST  #323, 0.5\n"
"6\tJMP  7\n"
"7\tSTORE_CONST  %0,  #301\n"
"8\tSTORE_CONST  %0,  #300\n"
"9\tEND\n"
"10\tSTORE_BOOL_CONST  #322, true\n";

    EXPECT_EQ(ss3.str(), expected3);

    delete bc;
}

TEST(BytecodeTransforms, Erasing) {
    Bytecode *bc = new Bytecode();

    bc->push_back(new opcode::JmpIfTrue(300, 8));
    bc->push_back(new opcode::Jmp(3));
    bc->push_back(new opcode::StoreIntConst(300, 7));
    bc->push_back(new opcode::StoreIntConst(301, 42));
    bc->push_back(new opcode::StoreIntConst(321, 426));
    bc->push_back(new opcode::BreakTo(6));
    bc->push_back(new opcode::StoreConst(0, 301));
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::End());

    bc->erase(4);
    std::stringstream ss;
    ss << *bc;

    ustring expected = 
"0\tJMP_IF_TRUE  %300, 7\n"
"1\tJMP  3\n"
"2\tSTORE_INT_CONST  #300, 7\n"
"3\tSTORE_INT_CONST  #301, 42\n"
"4\tBREAK_TO  5\n"
"5\tSTORE_CONST  %0,  #301\n"
"6\tSTORE_CONST  %0,  #300\n"
"7\tEND\n";

    EXPECT_EQ(ss.str(), expected);

    bc->erase(0);
    std::stringstream ss2;
    ss2 << *bc;

    ustring expected2 = 
"0\tJMP  2\n"
"1\tSTORE_INT_CONST  #300, 7\n"
"2\tSTORE_INT_CONST  #301, 42\n"
"3\tBREAK_TO  4\n"
"4\tSTORE_CONST  %0,  #301\n"
"5\tSTORE_CONST  %0,  #300\n"
"6\tEND\n";

    EXPECT_EQ(ss2.str(), expected2);

    bc->erase(5);
    std::stringstream ss3;
    ss3 << *bc;

    ustring expected3 = 
"0\tJMP  2\n"
"1\tSTORE_INT_CONST  #300, 7\n"
"2\tSTORE_INT_CONST  #301, 42\n"
"3\tBREAK_TO  4\n"
"4\tSTORE_CONST  %0,  #301\n"
"5\tEND\n";

    EXPECT_EQ(ss3.str(), expected3);

    delete bc;
}

}