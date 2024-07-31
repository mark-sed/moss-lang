#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <cmath>
#include "bytecode.hpp"
#include "values.hpp"
#include "testing_utils.hpp"

namespace{

using namespace moss;
using namespace testing;

TEST(Bytecode, Jmp) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(200, 7));
    bc->push_back(new opcode::StoreIntConst(201, 42));
    bc->push_back(new opcode::StoreConst(0, 201));
    bc->push_back(new opcode::Jmp(5));
    bc->push_back(new opcode::StoreConst(0, 200));
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
    bc->push_back(new opcode::StoreBoolConst(200, false));
    bc->push_back(new opcode::StoreIntConst(201, 42));
    bc->push_back(new opcode::StoreBoolConst(202, true));
    bc->push_back(new opcode::StoreIntConst(203, 14));
    bc->push_back(new opcode::StoreIntConst(204, 16));
    bc->push_back(new opcode::StoreConst(0, 200));
    bc->push_back(new opcode::StoreConst(1, 202));
    bc->push_back(new opcode::StoreConst(2, 201));
    bc->push_back(new opcode::JmpIfTrue(0, 13));
    bc->push_back(new opcode::JmpIfFalse(1, 13));
    bc->push_back(new opcode::StoreConst(2, 203));

    bc->push_back(new opcode::JmpIfTrue(1, 13));
    bc->push_back(new opcode::StoreConst(2, 204));
    bc->push_back(new opcode::StoreConst(3, 201)); //13
    bc->push_back(new opcode::JmpIfFalse(0, 16));
    bc->push_back(new opcode::StoreIntConst(3, 203));
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
TEST(Bytecode, Concat) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreStringConst(200, "Moss"));
    bc->push_back(new opcode::StoreStringConst(201, " "));
    bc->push_back(new opcode::StoreStringConst(202, "Language"));
    bc->push_back(new opcode::StoreStringConst(203, "!"));
    bc->push_back(new opcode::StoreConst(0, 200));
    bc->push_back(new opcode::Concat3(1, 0, 201));
    bc->push_back(new opcode::StoreConst(2, 203));
    bc->push_back(new opcode::Concat2(3, 202, 2));
    bc->push_back(new opcode::Concat(4, 1, 3));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(string_val(i->load(4)), ustring("Moss Language!"));

    delete i;
    delete bc;
}

TEST(Bytecode, Arithmetics) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(200, 2));
    bc->push_back(new opcode::StoreIntConst(201, 3));
    bc->push_back(new opcode::StoreIntConst(202, 9));
    bc->push_back(new opcode::StoreFloatConst(203, 0.5));
    bc->push_back(new opcode::StoreConst(0, 200));

    // Exp
    bc->push_back(new opcode::Exp3(1, 0, 201)); // 8
    bc->push_back(new opcode::StoreConst(2, 203));
    bc->push_back(new opcode::Exp2(3, 202, 2)); // 3.0
    bc->push_back(new opcode::Exp(4, 1, 3)); // 512.0.

    // Add
    bc->push_back(new opcode::Add(5, 4, 1)); // 520.0 
    bc->push_back(new opcode::Add3(6, 0, 202)); //11

    // Sub
    bc->push_back(new opcode::StoreIntConst(204, 280));
    bc->push_back(new opcode::Sub(7, 5, 1)); // 512.0 
    bc->push_back(new opcode::Sub2(8, 204, 6)); //269

    // Div
    bc->push_back(new opcode::Div(9, 7, 1)); // 64.0 
    bc->push_back(new opcode::Div2(10, 204, 1)); //35

    // Mul
    bc->push_back(new opcode::Mul(11, 7, 1)); // 4096.0
    bc->push_back(new opcode::Mul3(12, 1, 204)); //2240

    // Mod
    bc->push_back(new opcode::StoreFloatConst(205, 2.2));
    bc->push_back(new opcode::Mod(13, 7, 6)); // 6.0
    bc->push_back(new opcode::Mod2(14, 204, 1)); //0
    bc->push_back(new opcode::Mod3(15, 7, 205)); // 1.6...

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

    delete i;
    delete bc;
}

TEST(Bytecode, EqualsNotEquals) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(200, 42));
    bc->push_back(new opcode::StoreIntConst(201, 82));
    bc->push_back(new opcode::StoreIntConst(202, -42));

    bc->push_back(new opcode::StoreFloatConst(203, 0.0));
    bc->push_back(new opcode::StoreFloatConst(204, -99.0));
    bc->push_back(new opcode::StoreFloatConst(205, std::numeric_limits<double>::quiet_NaN()));
    bc->push_back(new opcode::StoreFloatConst(206, std::numeric_limits<double>::infinity()));
    bc->push_back(new opcode::StoreFloatConst(207, -std::numeric_limits<double>::infinity()));

    bc->push_back(new opcode::StoreBoolConst(208, true));
    bc->push_back(new opcode::StoreBoolConst(209, false));

    bc->push_back(new opcode::StoreStringConst(210, ""));
    bc->push_back(new opcode::StoreStringConst(211, "hi"));
    bc->push_back(new opcode::StoreStringConst(212, "hi2"));
    bc->push_back(new opcode::StoreStringConst(213, "🍣"));
    bc->push_back(new opcode::StoreStringConst(214, "ř"));

    bc->push_back(new opcode::StoreConst(0, 200));
    bc->push_back(new opcode::StoreConst(1, 201));
    bc->push_back(new opcode::StoreConst(2, 202));
    bc->push_back(new opcode::StoreConst(3, 203));
    bc->push_back(new opcode::StoreConst(4, 204));
    bc->push_back(new opcode::StoreConst(5, 205));
    bc->push_back(new opcode::StoreConst(6, 206));
    bc->push_back(new opcode::StoreConst(7, 207));
    bc->push_back(new opcode::StoreConst(8, 208));
    bc->push_back(new opcode::StoreConst(9, 209));
    bc->push_back(new opcode::StoreConst(10, 210));
    bc->push_back(new opcode::StoreConst(11, 211));
    bc->push_back(new opcode::StoreConst(12, 212));
    bc->push_back(new opcode::StoreConst(13, 213));
    bc->push_back(new opcode::StoreConst(14, 214));

    // Compare value to its own const
    bc->push_back(new opcode::Eq3(15, 0, 200));
    bc->push_back(new opcode::Eq3(16, 1, 201));
    bc->push_back(new opcode::Eq3(17, 2, 202));
    bc->push_back(new opcode::Eq3(18, 3, 203));
    bc->push_back(new opcode::Eq3(19, 4, 204));
    bc->push_back(new opcode::Eq3(20, 5, 205));
    bc->push_back(new opcode::Eq3(21, 6, 206));
    bc->push_back(new opcode::Eq3(22, 7, 207));
    bc->push_back(new opcode::Eq3(23, 8, 208));
    bc->push_back(new opcode::Eq3(24, 9, 209));
    bc->push_back(new opcode::Eq3(25, 10, 210));
    bc->push_back(new opcode::Eq3(26, 11, 211));
    bc->push_back(new opcode::Eq3(27, 12, 212));
    bc->push_back(new opcode::Eq3(28, 13, 213));
    bc->push_back(new opcode::Eq3(29, 14, 214));
    bc->push_back(new opcode::Eq2(30, 214, 14));
    bc->push_back(new opcode::Eq2(31, 210, 10));
    bc->push_back(new opcode::Eq2(32, 206, 6));

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
    bc->push_back(new opcode::StoreStringConst(215, "r"));
    bc->push_back(new opcode::Eq2(42, 215, 14));

    // Nil compare
    bc->push_back(new opcode::StoreNilConst(216));
    bc->push_back(new opcode::StoreConst(43, 216));

    bc->push_back(new opcode::Eq2(44, 216, 43));
    bc->push_back(new opcode::Eq(45, 43, 43));
    bc->push_back(new opcode::Eq(46, 43, 0));
    bc->push_back(new opcode::Eq(47, 43, 5));
    bc->push_back(new opcode::Eq(48, 43, 9));
    bc->push_back(new opcode::Eq(49, 43, 10));
    bc->push_back(new opcode::Eq(50, 43, 13));

    // NEq (calls Eq)
    bc->push_back(new opcode::NEq(51, 43, 13));
    bc->push_back(new opcode::NEq2(52, 216, 43));
    bc->push_back(new opcode::NEq3(53, 5, 205));

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

}