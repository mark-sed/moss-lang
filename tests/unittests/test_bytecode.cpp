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
using namespace testing;

TEST(Bytecode, Jmp) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, 7));
    bc->push_back(new opcode::StoreIntConst(301, 42));
    bc->push_back(new opcode::StoreConst(0, 301));
    bc->push_back(new opcode::Jmp(5));
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::End());

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(0)), 42);

    delete i;
    delete bc;
}

TEST(Bytecode, JmpIf) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreBoolConst(300, false));
    bc->push_back(new opcode::StoreIntConst(301, 42));
    bc->push_back(new opcode::StoreBoolConst(302, true));
    bc->push_back(new opcode::StoreIntConst(303, 14));
    bc->push_back(new opcode::StoreIntConst(304, 16));
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 302));
    bc->push_back(new opcode::StoreConst(2, 301));
    bc->push_back(new opcode::JmpIfTrue(0, 13));
    bc->push_back(new opcode::JmpIfFalse(1, 13));
    bc->push_back(new opcode::StoreConst(2, 303));

    bc->push_back(new opcode::JmpIfTrue(1, 13));
    bc->push_back(new opcode::StoreConst(2, 304));
    bc->push_back(new opcode::StoreConst(3, 301)); //13
    bc->push_back(new opcode::JmpIfFalse(0, 16));
    bc->push_back(new opcode::StoreIntConst(3, 303));
    bc->push_back(new opcode::End());

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(2)), 14);
    EXPECT_EQ(int_val(i->load(3)), 42);

    delete i;
    delete bc;
}

/** Tests Concat, Concat2 and Concat3 */
/*TEST(Bytecode, Concat) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreStringConst(304, "Moss"));
    bc->push_back(new opcode::StoreStringConst(301, " "));
    bc->push_back(new opcode::StoreStringConst(302, "Language"));
    bc->push_back(new opcode::StoreStringConst(303, "!"));
    bc->push_back(new opcode::StoreConst(100, 304));
    bc->push_back(new opcode::Concat3(101, 100, 301));
    bc->push_back(new opcode::StoreConst(102, 303));
    bc->push_back(new opcode::Concat2(103, 302, 102));
    bc->push_back(new opcode::Concat(104, 101, 103));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(string_val(i->load(104)), ustring("Moss Language!"));

    delete i;
    delete bc;
}

TEST(Bytecode, Arithmetics) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, 2));
    bc->push_back(new opcode::StoreIntConst(301, 3));
    bc->push_back(new opcode::StoreIntConst(302, 9));
    bc->push_back(new opcode::StoreFloatConst(303, 0.5));
    bc->push_back(new opcode::StoreConst(0, 300));

    // Exp
    bc->push_back(new opcode::Exp3(1, 0, 301)); // 8
    bc->push_back(new opcode::StoreConst(2, 303));
    bc->push_back(new opcode::Exp2(3, 302, 2)); // 3.0
    bc->push_back(new opcode::Exp(4, 1, 3)); // 512.0.

    // Add
    bc->push_back(new opcode::Add(5, 4, 1)); // 520.0 
    bc->push_back(new opcode::Add3(6, 0, 302)); //11

    // Sub
    bc->push_back(new opcode::StoreIntConst(304, 280));
    bc->push_back(new opcode::Sub(7, 5, 1)); // 512.0 
    bc->push_back(new opcode::Sub2(8, 304, 6)); //269

    // Div
    bc->push_back(new opcode::Div(9, 7, 1)); // 64.0 
    bc->push_back(new opcode::Div2(10, 304, 1)); //35

    // Mul
    bc->push_back(new opcode::Mul(11, 7, 1)); // 4096.0
    bc->push_back(new opcode::Mul3(12, 1, 304)); //2240

    // Mod
    bc->push_back(new opcode::StoreFloatConst(305, 2.2));
    bc->push_back(new opcode::Mod(13, 7, 6)); // 6.0
    bc->push_back(new opcode::Mod2(14, 304, 1)); //0
    bc->push_back(new opcode::Mod3(15, 7, 305)); // 1.6...

    // Neg
    bc->push_back(new opcode::Neg(16, 1)); // -8
    bc->push_back(new opcode::Neg(17, 2)); // -0.5 
    bc->push_back(new opcode::Neg(18, 16)); // 8 

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(1)), 8);
    EXPECT_EQ(float_val(i->load(3)), 3.0);
    EXPECT_EQ(float_val(i->load(4)), 512.0);

    EXPECT_EQ(float_val(i->load(5)), 520.0);
    EXPECT_EQ(int_val(i->load(6)), 11);

    EXPECT_EQ(float_val(i->load(7)), 512.0);
    EXPECT_EQ(int_val(i->load(8)), 269);

    EXPECT_EQ(float_val(i->load(9)), 64.0);
    EXPECT_EQ(int_val(i->load(10)), 35);

    EXPECT_EQ(float_val(i->load(11)), 4096.0);
    EXPECT_EQ(int_val(i->load(12)), 2240);

    EXPECT_EQ(float_val(i->load(13)), 6.0);
    EXPECT_EQ(int_val(i->load(14)), 0);
    EXPECT_EQ(float_val(i->load(15)), std::fmod(512, 2.2));

    EXPECT_EQ(int_val(i->load(16)), -8);
    EXPECT_EQ(float_val(i->load(17)), -0.5);
    EXPECT_EQ(int_val(i->load(18)), 8);

    delete i;
    delete bc;
}

TEST(Bytecode, EqualsNotEquals) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, 42));
    bc->push_back(new opcode::StoreIntConst(301, 82));
    bc->push_back(new opcode::StoreIntConst(302, -42));

    bc->push_back(new opcode::StoreFloatConst(303, 0.0));
    bc->push_back(new opcode::StoreFloatConst(304, -99.0));
    bc->push_back(new opcode::StoreFloatConst(305, std::numeric_limits<double>::quiet_NaN()));
    bc->push_back(new opcode::StoreFloatConst(306, std::numeric_limits<double>::infinity()));
    bc->push_back(new opcode::StoreFloatConst(307, -std::numeric_limits<double>::infinity()));

    bc->push_back(new opcode::StoreBoolConst(308, true));
    bc->push_back(new opcode::StoreBoolConst(309, false));

    bc->push_back(new opcode::StoreStringConst(310, ""));
    bc->push_back(new opcode::StoreStringConst(311, "hi"));
    bc->push_back(new opcode::StoreStringConst(312, "hi2"));
    bc->push_back(new opcode::StoreStringConst(313, "🍣"));
    bc->push_back(new opcode::StoreStringConst(314, "ř"));

    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 301));
    bc->push_back(new opcode::StoreConst(2, 302));
    bc->push_back(new opcode::StoreConst(3, 303));
    bc->push_back(new opcode::StoreConst(4, 304));
    bc->push_back(new opcode::StoreConst(5, 305));
    bc->push_back(new opcode::StoreConst(6, 306));
    bc->push_back(new opcode::StoreConst(7, 307));
    bc->push_back(new opcode::StoreConst(8, 308));
    bc->push_back(new opcode::StoreConst(9, 309));
    bc->push_back(new opcode::StoreConst(10, 310));
    bc->push_back(new opcode::StoreConst(11, 311));
    bc->push_back(new opcode::StoreConst(12, 312));
    bc->push_back(new opcode::StoreConst(13, 313));
    bc->push_back(new opcode::StoreConst(14, 314));

    // Compare value to its own const
    bc->push_back(new opcode::Eq3(15, 0, 300));
    bc->push_back(new opcode::Eq3(16, 1, 301));
    bc->push_back(new opcode::Eq3(17, 2, 302));
    bc->push_back(new opcode::Eq3(18, 3, 303));
    bc->push_back(new opcode::Eq3(19, 4, 304));
    bc->push_back(new opcode::Eq3(20, 5, 305));
    bc->push_back(new opcode::Eq3(21, 6, 306));
    bc->push_back(new opcode::Eq3(22, 7, 307));
    bc->push_back(new opcode::Eq3(23, 8, 308));
    bc->push_back(new opcode::Eq3(24, 9, 309));
    bc->push_back(new opcode::Eq3(25, 10, 310));
    bc->push_back(new opcode::Eq3(26, 11, 311));
    bc->push_back(new opcode::Eq3(27, 12, 312));
    bc->push_back(new opcode::Eq3(28, 13, 313));
    bc->push_back(new opcode::Eq3(29, 14, 314));
    bc->push_back(new opcode::Eq2(30, 314, 14));
    bc->push_back(new opcode::Eq2(31, 310, 10));
    bc->push_back(new opcode::Eq2(32, 306, 6));

    // Similar values but not the same
    bc->push_back(new opcode::Eq(33, 0, 2));
    bc->push_back(new opcode::Eq(34, 3, 4));
    bc->push_back(new opcode::Eq(35, 5, 6));
    bc->push_back(new opcode::Eq(36, 7, 6));
    bc->push_back(new opcode::Eq(37, 8, 9));
    bc->push_back(new opcode::Eq(38, 10, 11));
    bc->push_back(new opcode::Eq(39, 11, 12));
    bc->push_back(new opcode::Eq(40, 12, 13));
    bc->push_back(new opcode::Eq(41, 13, 14));
    bc->push_back(new opcode::StoreStringConst(315, "r"));
    bc->push_back(new opcode::Eq2(42, 315, 14));

    // Nil compare
    bc->push_back(new opcode::StoreNilConst(316));
    bc->push_back(new opcode::StoreConst(43, 316));

    bc->push_back(new opcode::Eq2(44, 316, 43));
    bc->push_back(new opcode::Eq(45, 43, 43));
    bc->push_back(new opcode::Eq(46, 43, 0));
    bc->push_back(new opcode::Eq(47, 43, 5));
    bc->push_back(new opcode::Eq(48, 43, 9));
    bc->push_back(new opcode::Eq(49, 43, 10));
    bc->push_back(new opcode::Eq(50, 43, 13));

    // Neq (calls Eq)
    bc->push_back(new opcode::Neq(51, 43, 13));
    bc->push_back(new opcode::Neq2(52, 316, 43));
    bc->push_back(new opcode::Neq3(53, 5, 305));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);

    EXPECT_EQ(bool_val(i->load(15)), true);
    EXPECT_EQ(bool_val(i->load(16)), true);
    EXPECT_EQ(bool_val(i->load(17)), true);
    EXPECT_EQ(bool_val(i->load(18)), true);
    EXPECT_EQ(bool_val(i->load(19)), true);
    EXPECT_EQ(bool_val(i->load(20)), false); // nan
    EXPECT_EQ(bool_val(i->load(21)), true);
    EXPECT_EQ(bool_val(i->load(22)), true);
    EXPECT_EQ(bool_val(i->load(23)), true);
    EXPECT_EQ(bool_val(i->load(24)), true);
    EXPECT_EQ(bool_val(i->load(25)), true);
    EXPECT_EQ(bool_val(i->load(26)), true);
    EXPECT_EQ(bool_val(i->load(27)), true);
    EXPECT_EQ(bool_val(i->load(28)), true);
    EXPECT_EQ(bool_val(i->load(29)), true);
    EXPECT_EQ(bool_val(i->load(30)), true);
    EXPECT_EQ(bool_val(i->load(31)), true);
    EXPECT_EQ(bool_val(i->load(32)), true);

    EXPECT_EQ(bool_val(i->load(33)), false);
    EXPECT_EQ(bool_val(i->load(34)), false);
    EXPECT_EQ(bool_val(i->load(35)), false);
    EXPECT_EQ(bool_val(i->load(36)), false);
    EXPECT_EQ(bool_val(i->load(37)), false);
    EXPECT_EQ(bool_val(i->load(38)), false);
    EXPECT_EQ(bool_val(i->load(39)), false);
    EXPECT_EQ(bool_val(i->load(40)), false);
    EXPECT_EQ(bool_val(i->load(41)), false);
    EXPECT_EQ(bool_val(i->load(42)), false);

    EXPECT_EQ(bool_val(i->load(44)), true);
    EXPECT_EQ(bool_val(i->load(45)), true);
    EXPECT_EQ(bool_val(i->load(46)), false);
    EXPECT_EQ(bool_val(i->load(47)), false);
    EXPECT_EQ(bool_val(i->load(48)), false);
    EXPECT_EQ(bool_val(i->load(49)), false);
    EXPECT_EQ(bool_val(i->load(50)), false);

    EXPECT_EQ(bool_val(i->load(51)), true);
    EXPECT_EQ(bool_val(i->load(52)), false);
    EXPECT_EQ(bool_val(i->load(53)), true);

    delete i;
    delete bc;
}

TEST(Bytecode, Comparisons) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, -9922));
    bc->push_back(new opcode::StoreIntConst(301, -9923));
    bc->push_back(new opcode::StoreIntConst(302, 999));

    bc->push_back(new opcode::StoreFloatConst(303, 0.0));
    bc->push_back(new opcode::StoreFloatConst(304, 0.0001));
    bc->push_back(new opcode::StoreFloatConst(305, -99.0));
    bc->push_back(new opcode::StoreFloatConst(306, std::numeric_limits<double>::quiet_NaN()));
    bc->push_back(new opcode::StoreFloatConst(307, std::numeric_limits<double>::infinity()));
    bc->push_back(new opcode::StoreFloatConst(308, -std::numeric_limits<double>::infinity()));

    bc->push_back(new opcode::StoreStringConst(309, "kača"));
    bc->push_back(new opcode::StoreStringConst(310, "kaca"));

    bc->push_back(new opcode::StoreStringConst(311, ""));
    bc->push_back(new opcode::StoreStringConst(312, "hi"));
    bc->push_back(new opcode::StoreStringConst(313, "hi2"));
    bc->push_back(new opcode::StoreStringConst(314, "abcdefg"));
    bc->push_back(new opcode::StoreStringConst(315, "abcdegg"));

    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 301));
    bc->push_back(new opcode::StoreConst(2, 302));
    bc->push_back(new opcode::StoreConst(3, 303));
    bc->push_back(new opcode::StoreConst(4, 304));
    bc->push_back(new opcode::StoreConst(5, 305));
    bc->push_back(new opcode::StoreConst(6, 306));
    bc->push_back(new opcode::StoreConst(7, 307));
    bc->push_back(new opcode::StoreConst(8, 308));
    bc->push_back(new opcode::StoreConst(9, 309));
    bc->push_back(new opcode::StoreConst(10, 310));
    bc->push_back(new opcode::StoreConst(11, 311));
    bc->push_back(new opcode::StoreConst(12, 312));
    bc->push_back(new opcode::StoreConst(13, 313));
    bc->push_back(new opcode::StoreConst(14, 314));
    bc->push_back(new opcode::StoreConst(15, 315));

    // Bt
    bc->push_back(new opcode::Bt2(16, 300, 0));
    bc->push_back(new opcode::Bt3(17, 0, 301));
    bc->push_back(new opcode::Bt(18, 1, 0));

    bc->push_back(new opcode::Bt(19, 4, 3));
    bc->push_back(new opcode::Bt(20, 3, 5));
    bc->push_back(new opcode::Bt2(21, 3206, 6));
    bc->push_back(new opcode::Bt(22, 7, 6));
    bc->push_back(new opcode::Bt3(23, 7, 307));
    bc->push_back(new opcode::Bt(24, 7, 8));

    bc->push_back(new opcode::Bt(25, 9, 10));
    bc->push_back(new opcode::Bt(26, 9, 11));
    bc->push_back(new opcode::Bt(27, 13, 12));
    bc->push_back(new opcode::Bt(28, 15, 14));

    // Lt
    bc->push_back(new opcode::Lt2(29, 300, 0));
    bc->push_back(new opcode::Lt3(30, 0, 301));
    bc->push_back(new opcode::Lt(31, 1, 0));

    bc->push_back(new opcode::Lt(32, 4, 3));
    bc->push_back(new opcode::Lt(33, 3, 5));
    bc->push_back(new opcode::Lt2(34, 306, 6));
    bc->push_back(new opcode::Lt(35, 7, 6));
    bc->push_back(new opcode::Lt3(36, 7, 307));
    bc->push_back(new opcode::Lt(37, 7, 8));

    bc->push_back(new opcode::Lt(38, 9, 10));
    bc->push_back(new opcode::Lt(39, 9, 11));
    bc->push_back(new opcode::Lt(40, 13, 12));
    bc->push_back(new opcode::Lt(41, 15, 14));

    // Beq
    bc->push_back(new opcode::Beq3(42, 0, 301));
    bc->push_back(new opcode::Beq2(43, 300, 0));
    bc->push_back(new opcode::Beq(44, 7, 7));

    // Leq
    bc->push_back(new opcode::Leq3(45, 7, 307));
    bc->push_back(new opcode::Leq(46, 13, 12));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(bool_val(i->load(16)), false);
    EXPECT_EQ(bool_val(i->load(17)), true);
    EXPECT_EQ(bool_val(i->load(18)), false);

    EXPECT_EQ(bool_val(i->load(19)), true);
    EXPECT_EQ(bool_val(i->load(20)), true);
    EXPECT_EQ(bool_val(i->load(21)), false);
    EXPECT_EQ(bool_val(i->load(22)), false);
    EXPECT_EQ(bool_val(i->load(23)), false);
    EXPECT_EQ(bool_val(i->load(24)), true);

    //TODO: Reenable 
    //EXPECT_EQ(bool_val(i->load(25)), true);
    EXPECT_EQ(bool_val(i->load(26)), true);
    EXPECT_EQ(bool_val(i->load(27)), true);
    EXPECT_EQ(bool_val(i->load(28)), true);

    EXPECT_EQ(bool_val(i->load(29)), false);
    EXPECT_EQ(bool_val(i->load(30)), false);
    EXPECT_EQ(bool_val(i->load(31)), true);

    EXPECT_EQ(bool_val(i->load(32)), false);
    EXPECT_EQ(bool_val(i->load(33)), false);
    EXPECT_EQ(bool_val(i->load(34)), false);
    EXPECT_EQ(bool_val(i->load(35)), false);
    EXPECT_EQ(bool_val(i->load(36)), false);
    EXPECT_EQ(bool_val(i->load(37)), false);

    //EXPECT_EQ(bool_val(i->load(38)), false);
    EXPECT_EQ(bool_val(i->load(39)), false);
    EXPECT_EQ(bool_val(i->load(40)), false);
    EXPECT_EQ(bool_val(i->load(41)), false);

    EXPECT_EQ(bool_val(i->load(42)), true);
    EXPECT_EQ(bool_val(i->load(43)), true);
    EXPECT_EQ(bool_val(i->load(44)), true);

    EXPECT_EQ(bool_val(i->load(45)), true);
    EXPECT_EQ(bool_val(i->load(46)), false);

    delete i;
    delete bc;
}

TEST(Bytecode, In) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreStringConst(300, "Moss"));
    bc->push_back(new opcode::StoreStringConst(301, " "));
    bc->push_back(new opcode::StoreStringConst(302, "Language"));
    bc->push_back(new opcode::StoreStringConst(303, ""));
    bc->push_back(new opcode::StoreStringConst(304, "gu"));
    bc->push_back(new opcode::StoreStringConst(305, "os"));
    
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 301));
    bc->push_back(new opcode::StoreConst(2, 302));
    bc->push_back(new opcode::StoreConst(3, 303));
    bc->push_back(new opcode::StoreConst(4, 304));
    bc->push_back(new opcode::StoreConst(5, 305));

    bc->push_back(new opcode::In3(6, 0, 302));
    bc->push_back(new opcode::In2(7, 300, 0));
    bc->push_back(new opcode::In(8, 4, 2));
    bc->push_back(new opcode::In(9, 2, 4));
    bc->push_back(new opcode::In(10, 3, 1));
    bc->push_back(new opcode::In(11, 5, 0));
    bc->push_back(new opcode::In2(12, 302, 2));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);

    EXPECT_EQ(bool_val(i->load(6)), false);
    EXPECT_EQ(bool_val(i->load(7)), true);
    EXPECT_EQ(bool_val(i->load(8)), true);
    EXPECT_EQ(bool_val(i->load(9)), false);
    EXPECT_EQ(bool_val(i->load(10)), true);
    EXPECT_EQ(bool_val(i->load(11)), true);
    EXPECT_EQ(bool_val(i->load(12)), true);

    delete i;
    delete bc;
}

TEST(Bytecode, BitWiseOperators) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, 0));
    bc->push_back(new opcode::StoreIntConst(301, -1));
    bc->push_back(new opcode::StoreIntConst(302, 0xF0));
    bc->push_back(new opcode::StoreIntConst(303, 0xF));
    bc->push_back(new opcode::StoreBoolConst(304, true));
    bc->push_back(new opcode::StoreBoolConst(305, false));
    
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 301));
    bc->push_back(new opcode::StoreConst(2, 302));
    bc->push_back(new opcode::StoreConst(3, 303));
    bc->push_back(new opcode::StoreConst(4, 304));
    bc->push_back(new opcode::StoreConst(5, 305));

    // And
    bc->push_back(new opcode::And3(6, 0, 302));
    bc->push_back(new opcode::And2(7, 303, 1));
    bc->push_back(new opcode::And(8, 3, 2));
    bc->push_back(new opcode::And(9, 4, 4));
    bc->push_back(new opcode::And(10, 4, 5));
    bc->push_back(new opcode::And(11, 5, 5));

    // Or
    bc->push_back(new opcode::Or3(12, 0, 302));
    bc->push_back(new opcode::Or2(13, 303, 2));
    bc->push_back(new opcode::Or(14, 1, 0));
    bc->push_back(new opcode::Or2(15, 300, 0));
    bc->push_back(new opcode::Or3(16, 4, 304));
    bc->push_back(new opcode::Or(17, 4, 5));
    bc->push_back(new opcode::Or2(18, 305, 5));

    // Xor
    bc->push_back(new opcode::Xor3(19, 3, 301));
    bc->push_back(new opcode::Xor2(20, 303, 3));
    bc->push_back(new opcode::Xor(21, 2, 3));
    bc->push_back(new opcode::Xor2(22, 304, 4));
    bc->push_back(new opcode::Xor3(23, 5, 305));
    bc->push_back(new opcode::Xor(24, 5, 4));
    bc->push_back(new opcode::Xor(25, 4, 5));

    // Not
    bc->push_back(new opcode::Not(26, 0));
    bc->push_back(new opcode::Not(27, 1));
    bc->push_back(new opcode::Not(28, 4));
    bc->push_back(new opcode::Not(29, 5));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);

    EXPECT_EQ(int_val(i->load(6)), 0);
    EXPECT_EQ(int_val(i->load(7)), 0xF);
    EXPECT_EQ(int_val(i->load(8)), 0);
    EXPECT_EQ(bool_val(i->load(9)), true);
    EXPECT_EQ(bool_val(i->load(10)), false);
    EXPECT_EQ(bool_val(i->load(11)), false);

    EXPECT_EQ(int_val(i->load(12)), 0xF0);
    EXPECT_EQ(int_val(i->load(13)), 0xFF);
    EXPECT_EQ(int_val(i->load(14)), -1);
    EXPECT_EQ(int_val(i->load(15)), 0);
    EXPECT_EQ(bool_val(i->load(16)), true);
    EXPECT_EQ(bool_val(i->load(17)), true);
    EXPECT_EQ(bool_val(i->load(18)), false);

    EXPECT_EQ(int_val(i->load(19)), -16);
    EXPECT_EQ(int_val(i->load(20)), 0);
    EXPECT_EQ(int_val(i->load(21)), 0xFF);
    EXPECT_EQ(bool_val(i->load(22)), false);
    EXPECT_EQ(bool_val(i->load(23)), false);
    EXPECT_EQ(bool_val(i->load(24)), true);
    EXPECT_EQ(bool_val(i->load(25)), true);

    EXPECT_EQ(int_val(i->load(26)), -1);
    EXPECT_EQ(int_val(i->load(27)), 0);
    EXPECT_EQ(bool_val(i->load(28)), false);
    EXPECT_EQ(bool_val(i->load(29)), true);

    delete i;
    delete bc;
}

TEST(Bytecode, Subscript) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreStringConst(300, "Moss"));
    bc->push_back(new opcode::StoreStringConst(301, " "));
    bc->push_back(new opcode::StoreStringConst(302, "Language"));
    bc->push_back(new opcode::StoreIntConst(303, 0));
    bc->push_back(new opcode::StoreIntConst(304, 2));
    bc->push_back(new opcode::StoreIntConst(305, 7));
    
    bc->push_back(new opcode::StoreConst(0, 300));
    bc->push_back(new opcode::StoreConst(1, 301));
    bc->push_back(new opcode::StoreConst(2, 302));
    bc->push_back(new opcode::StoreConst(3, 303));
    bc->push_back(new opcode::StoreConst(4, 304));
    bc->push_back(new opcode::StoreConst(5, 305));

    bc->push_back(new opcode::Subsc3(6, 0, 303));
    bc->push_back(new opcode::Subsc2(7, 300, 4));
    bc->push_back(new opcode::Subsc(8, 1, 3));
    bc->push_back(new opcode::Subsc(9, 2, 3));
    bc->push_back(new opcode::Subsc(10, 2, 4));
    bc->push_back(new opcode::Subsc(11, 2, 5));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);

    EXPECT_EQ(string_val(i->load(6)), "M");
    EXPECT_EQ(string_val(i->load(7)), "s");
    EXPECT_EQ(string_val(i->load(8)), " ");
    EXPECT_EQ(string_val(i->load(9)), "L");
    EXPECT_EQ(string_val(i->load(10)), "n");
    EXPECT_EQ(string_val(i->load(11)), "e");

    delete i;
    delete bc;
}*/

}