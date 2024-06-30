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
    bc->push_back(new opcode::StoreConst(2, 203)); // <-

    bc->push_back(new opcode::JmpIfTrue(1, 13));
    bc->push_back(new opcode::StoreConst(2, 204));
    bc->push_back(new opcode::StoreConst(3, 201)); //13
    bc->push_back(new opcode::JmpIfFalse(0, 16));
    bc->push_back(new opcode::StoreIntConst(3, 203)); //10
    bc->push_back(new opcode::End());

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(2)), 14);
    EXPECT_EQ(int_val(i->load(3)), 42);

    delete i;
    delete bc;
}

}