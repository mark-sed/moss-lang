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

    auto file_path = "/tmp/mosstest.msb";

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

}