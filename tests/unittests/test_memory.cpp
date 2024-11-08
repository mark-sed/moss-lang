#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <cmath>
#include "bytecode.hpp"
#include "values.hpp"
#include "testing_utils.hpp"

namespace {

using namespace moss;
using namespace testing;

TEST(Memory, Attributes) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(300, 42));
    bc->push_back(new opcode::StoreIntConst(301, 24));
    bc->push_back(new opcode::StoreConst(100, 300));
    bc->push_back(new opcode::StoreConst(101, 301));
    bc->push_back(new opcode::StoreAttr(101, 100, "some_val"));
    bc->push_back(new opcode::LoadAttr(102, 100, "some_val"));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(100)), 42);
    EXPECT_EQ(int_val(i->load(101)), 24);
    EXPECT_EQ(int_val(i->load(102)), 24);
    EXPECT_EQ(int_val(i->load_const(300)), 42);
    EXPECT_EQ(int_val(i->load_const(301)), 24);

    delete i;
    delete bc;
}

TEST(Memory, Constants) {
    Bytecode *bc = new Bytecode();
    int reg_start = 200;
    int reg = reg_start;
    bc->push_back(new opcode::StoreIntConst(reg++, -9223372036854775807));
    bc->push_back(new opcode::StoreIntConst(reg++, 9223372036854775807));
    bc->push_back(new opcode::StoreIntConst(reg++, 0));
    
    bc->push_back(new opcode::StoreFloatConst(reg++, 0.0));
    bc->push_back(new opcode::StoreFloatConst(reg++, std::numeric_limits<double>::infinity()));
    bc->push_back(new opcode::StoreFloatConst(reg++, -std::numeric_limits<double>::infinity()));
    bc->push_back(new opcode::StoreFloatConst(reg++, -std::numeric_limits<double>::quiet_NaN()));

    bc->push_back(new opcode::StoreBoolConst(reg++, true));
    bc->push_back(new opcode::StoreBoolConst(reg++, false));

    bc->push_back(new opcode::StoreNilConst(reg++));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(int_val(i->load_const(reg_start++)), -9223372036854775807LL);
    EXPECT_EQ(int_val(i->load_const(reg_start++)), 9223372036854775807LL);
    EXPECT_EQ(int_val(i->load_const(reg_start++)), 0);

    EXPECT_EQ(float_val(i->load_const(reg_start++)), 0.0);
    EXPECT_EQ(float_val(i->load_const(reg_start++)), std::numeric_limits<double>::infinity());
    EXPECT_EQ(float_val(i->load_const(reg_start++)), -std::numeric_limits<double>::infinity());
    EXPECT_TRUE(std::isnan(float_val(i->load_const(reg_start++))));

    EXPECT_EQ(bool_val(i->load_const(reg_start++)), true);
    EXPECT_EQ(bool_val(i->load_const(reg_start++)), false);

    EXPECT_TRUE(isa<NilValue>(i->load_const(reg_start++)));

    delete i;
    delete bc;
}

}