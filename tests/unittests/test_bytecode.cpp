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
    bc->push_back(new opcode::Jmp(4));
    bc->push_back(new opcode::StoreIntConst(201, 14));
    bc->push_back(new opcode::End());

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load_const(201)), 42);

    delete i;
    delete bc;
}

}