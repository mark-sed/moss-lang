#include <gtest/gtest.h>
#include <cstdint>
#include "bytecode.hpp"
#include "values.hpp"

namespace{

using namespace moss;

int64_t int_val(Value *v) {
    IntValue *iv = dyn_cast<IntValue>(v);
    EXPECT_TRUE(iv != nullptr) << "Value is not an integer";
    if(!iv) return 0;
    return iv->get_value();
}

TEST(Memory, Attributes){
    Bytecode *bc = new Bytecode();
    bc->push_back(new opcode::StoreIntConst(200, 42));
    bc->push_back(new opcode::StoreIntConst(201, 24));
    bc->push_back(new opcode::StoreConst(0, 200));
    bc->push_back(new opcode::StoreConst(1, 201));
    bc->push_back(new opcode::StoreAttr(1, 0, "some_val"));
    bc->push_back(new opcode::LoadAttr(2, 0, "some_val"));

    Interpreter *i = new Interpreter(bc);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);
    EXPECT_EQ(int_val(i->load(0)), 42);
    EXPECT_EQ(int_val(i->load(1)), 24);
    EXPECT_EQ(int_val(i->load(2)), 24);
    EXPECT_EQ(int_val(i->load_const(200)), 42);
    EXPECT_EQ(int_val(i->load_const(201)), 24);

    delete i;
    delete bc;
}

}