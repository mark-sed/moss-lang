#include <gtest/gtest.h>
#include "bytecode_reader.hpp"
#include "bytecode_writer.hpp"

namespace{

using namespace moss;

TEST(BytecodeWriterAndReader, TestCorrectness){
    Bytecode *bc = new Bytecode();
    /*
    STORE_INT_CONST #200, 2024      ; 13 200 2024
    STORE_CONST     %0, #200        ; 8 0 200
    STORE_NAME      %0, "foo"       ; 5 0 3"foo"
    LOAD            %1, "foo"       ; 1 1 3"foo"
    */
    bc->push_back(new opcode::StoreIntConst(200, 2024));
    bc->push_back(new opcode::StoreConst(0, 200));
    bc->push_back(new opcode::StoreName(0, "foo"));
    bc->push_back(new opcode::Load(1, "foo"));

#ifdef __linux__
    auto file_path = "/tmp/mosstest.msb";
#else
    auto file_path = "mosstest.msb";
#endif

    BytecodeFile bfo(file_path);
    BytecodeWriter *bcwriter = new BytecodeWriter(bfo);
    bcwriter->write(bc);

    BytecodeFile bf(file_path);
    BytecodeReader *bcreader = new BytecodeReader(bf);
    Bytecode *bc_read = bcreader->read();

    ASSERT_EQ(bc->size(), bc_read->size());
    for (unsigned int i = 0; i < bc->size(); ++i) {
        EXPECT_TRUE(*bc->get_code()[i] == *bc_read->get_code()[i]);
    }

    delete bc;
    delete bc_read;
    delete bcwriter;
    delete bcreader;
}

// Test for all opcodes
TEST(BytecodeWriterAndReader, AllOpCodes){
    Bytecode *bc = new Bytecode();
    // This bytecode is incorrect and should not be interpreted nor verified

    bc->push_back(new opcode::End());

    bc->push_back(new opcode::Load(0, "some_name"));
    bc->push_back(new opcode::LoadAttr(1, 0, "some_attr"));
    bc->push_back(new opcode::LoadGlobal(0, "some_name"));
    bc->push_back(new opcode::LoadNonLoc(0, "some_name"));

    bc->push_back(new opcode::Store(2, 0));
    bc->push_back(new opcode::StoreName(2, "some_name2"));
    bc->push_back(new opcode::StoreConst(3, 200));
    bc->push_back(new opcode::StoreAttr(0, 1, "aname"));
    bc->push_back(new opcode::StoreConstAttr(1, 2, "aaa"));
    bc->push_back(new opcode::StoreGlobal(8, "a1"));
    bc->push_back(new opcode::StoreNonLoc(8, "a2"));
    bc->push_back(new opcode::StoreSubsc(7, 8, 9));
    bc->push_back(new opcode::StoreConstSubsc(17, 8, 9));
    bc->push_back(new opcode::StoreSubscConst(7, 8, 19));
    bc->push_back(new opcode::StoreConstSubscConst(18, 8, 20));

    bc->push_back(new opcode::StoreIntConst(4, 0xFFFFFFFF));
    bc->push_back(new opcode::StoreFloatConst(5, 0.0e-8));
    bc->push_back(new opcode::StoreBoolConst(6, true));
    bc->push_back(new opcode::StoreStringConst(9, "string value\n"));
    bc->push_back(new opcode::StoreNilConst(7));

    bc->push_back(new opcode::Jmp(5));
    bc->push_back(new opcode::JmpIfTrue(2, -7));
    bc->push_back(new opcode::JmpIfFalse(3, -12));
    bc->push_back(new opcode::Call(11, 7));
    bc->push_back(new opcode::CallFormatter(11, "md"));
    bc->push_back(new opcode::PushFrame());
    bc->push_back(new opcode::PopFrame());
    bc->push_back(new opcode::PushCallFrame());
    bc->push_back(new opcode::PopCallFrame());
    bc->push_back(new opcode::Return(12));
    bc->push_back(new opcode::ReturnConst(13));
    bc->push_back(new opcode::PushArg(11));
    bc->push_back(new opcode::PushConstArg(200));
    bc->push_back(new opcode::PushNamedArg(5, "name12"));
    bc->push_back(new opcode::PushUnpacked(15));

    bc->push_back(new opcode::CreateFun(50, "foo", "a,b,c"));
    bc->push_back(new opcode::FunBegin(50));
    bc->push_back(new opcode::SetDefault(50, 0, 2));
    bc->push_back(new opcode::SetDefaultConst(50, 1, 3));
    bc->push_back(new opcode::SetType(50, 0, 5));
    bc->push_back(new opcode::SetVararg(50, 2));

    bc->push_back(new opcode::Import(18, "module1"));
    bc->push_back(new opcode::ImportAll(18));

    bc->push_back(new opcode::PushParent(11));
    bc->push_back(new opcode::BuildClass(0, "MyClass"));
    
    bc->push_back(new opcode::Annotate(12, "annt", 5));
    bc->push_back(new opcode::AnnotateMod("ann5", 5));
    bc->push_back(new opcode::Document(12, "some fun"));

    bc->push_back(new opcode::Output(3));
    
    bc->push_back(new opcode::Concat(0, 1, 2));
    bc->push_back(new opcode::Exp(0, 1, 2));
    bc->push_back(new opcode::Add(0, 1, 2));
    bc->push_back(new opcode::Sub(0, 1, 2));
    bc->push_back(new opcode::Div(0, 1, 2));
    bc->push_back(new opcode::Mul(0, 1, 2));
    bc->push_back(new opcode::Mod(0, 1, 2));
    bc->push_back(new opcode::Eq(0, 1, 2));
    bc->push_back(new opcode::Neq(0, 1, 2));
    bc->push_back(new opcode::Bt(0, 1, 2));
    bc->push_back(new opcode::Lt(0, 1, 2));
    bc->push_back(new opcode::Beq(0, 1, 2));
    bc->push_back(new opcode::Leq(0, 1, 2));
    bc->push_back(new opcode::In(0, 1, 2));
    bc->push_back(new opcode::And(0, 1, 2));
    bc->push_back(new opcode::Or(0, 1, 2));
    bc->push_back(new opcode::Xor(0, 1, 2));
    bc->push_back(new opcode::Subsc(0, 1, 2));

    bc->push_back(new opcode::Concat2(0, 1, 2));
    bc->push_back(new opcode::Exp2(0, 1, 2));
    bc->push_back(new opcode::Add2(0, 1, 2));
    bc->push_back(new opcode::Sub2(0, 1, 2));
    bc->push_back(new opcode::Div2(0, 1, 2));
    bc->push_back(new opcode::Mul2(0, 1, 2));
    bc->push_back(new opcode::Mod2(0, 1, 2));
    bc->push_back(new opcode::Eq2(0, 1, 2));
    bc->push_back(new opcode::Neq2(0, 1, 2));
    bc->push_back(new opcode::Bt2(0, 1, 2));
    bc->push_back(new opcode::Lt2(0, 1, 2));
    bc->push_back(new opcode::Beq2(0, 1, 2));
    bc->push_back(new opcode::Leq2(0, 1, 2));
    bc->push_back(new opcode::In2(0, 1, 2));
    bc->push_back(new opcode::And2(0, 1, 2));
    bc->push_back(new opcode::Or2(0, 1, 2));
    bc->push_back(new opcode::Xor2(0, 1, 2));
    bc->push_back(new opcode::Subsc2(0, 1, 2));

    bc->push_back(new opcode::Concat3(0, 1, 2));
    bc->push_back(new opcode::Exp3(0, 1, 2));
    bc->push_back(new opcode::Add3(0, 1, 2));
    bc->push_back(new opcode::Sub3(0, 1, 2));
    bc->push_back(new opcode::Div3(0, 1, 2));
    bc->push_back(new opcode::Mul3(0, 1, 2));
    bc->push_back(new opcode::Mod3(0, 1, 2));
    bc->push_back(new opcode::Eq3(0, 1, 2));
    bc->push_back(new opcode::Neq3(0, 1, 2));
    bc->push_back(new opcode::Bt3(0, 1, 2));
    bc->push_back(new opcode::Lt3(0, 1, 2));
    bc->push_back(new opcode::Beq3(0, 1, 2));
    bc->push_back(new opcode::Leq3(0, 1, 2));
    bc->push_back(new opcode::In3(0, 1, 2));
    bc->push_back(new opcode::And3(0, 1, 2));
    bc->push_back(new opcode::Or3(0, 1, 2));
    bc->push_back(new opcode::Xor3(0, 1, 2));
    bc->push_back(new opcode::Subsc3(0, 1, 2));
    bc->push_back(new opcode::SubscLast(0, 1, 2));
    bc->push_back(new opcode::SubscRest(0, 1, 2));

    bc->push_back(new opcode::Not(0, 1));
    bc->push_back(new opcode::Neg(0, 1));
    
    bc->push_back(new opcode::Assert(7, 4));

    bc->push_back(new opcode::Raise(14));
    bc->push_back(new opcode::Catch("e", 204));
    bc->push_back(new opcode::CatchTyped("e", 5, 206));
    bc->push_back(new opcode::PopCatch());

    bc->push_back(new opcode::ListPush(7, 5));
    bc->push_back(new opcode::ListPushConst(7, 6));

    bc->push_back(new opcode::BuildList(8));

    bc->push_back(new opcode::BuildDict(4, 13, 14));

    bc->push_back(new opcode::BuildEnum(5, 19, "MyEnum"));

    bc->push_back(new opcode::BuildSpace(9, "MySpace"));

    bc->push_back(new opcode::CreateRange(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange2(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange3(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange4(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange5(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange6(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange7(1, 2, 3, 4));
    bc->push_back(new opcode::CreateRange8(1, 2, 3, 4));

    bc->push_back(new opcode::Switch(0, 4, 5, -3));
    bc->push_back(new opcode::For(10, 11, 55));
    bc->push_back(new opcode::ForMulti(10, 11, 55, 66));
    bc->push_back(new opcode::Iter(11, 14));

#ifdef __linux__
    auto file_path = "/tmp/mosstest_all.msb";
#else
    auto file_path = "mosstest_all.msb";
#endif

    BytecodeFile bfo(file_path);
    BytecodeWriter *bcwriter = new BytecodeWriter(bfo);
    bcwriter->write(bc);

    BytecodeFile bf(file_path);
    BytecodeReader *bcreader = new BytecodeReader(bf);
    Bytecode *bc_read = bcreader->read();

    ASSERT_EQ(bc->size(), static_cast<unsigned>(opcode::OpCodes::OPCODES_AMOUNT)) << "Not all opcodes are being tested";
    ASSERT_EQ(bc->size(), bc_read->size());
    for (unsigned int i = 0; i < bc->size(); ++i) {
        EXPECT_TRUE(*bc->get_code()[i] == *bc_read->get_code()[i]) << "Written: \'" << *(bc->get_code()[i]) << "'\n   Read: '" << *(bc_read->get_code()[i]) << "'";
        EXPECT_EQ(static_cast<unsigned int>(bc_read->get_code()[i]->get_type()), i) << "Read: '" << *(bc_read->get_code()[i]) << "'";
    }

    delete bc;
    delete bc_read;
    delete bcwriter;
    delete bcreader;
}

}