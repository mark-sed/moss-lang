#include <gtest/gtest.h>
#include <sstream>
#include "bytecode_blob.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"
#include "values.hpp"
#include "parser.hpp"
#include "bytecodegen.hpp"
#include "testing_utils.hpp"

namespace{

using namespace moss;
using namespace opcode;
using namespace testing;

std::string get_bcb_mnems(BCBlob &bcb) {
    std::stringstream ss;
    for (auto o: bcb) {
        ss << o->get_mnem() << "\n";
    }
    return ss.str();
}

TEST(BytecodeBlob, BlobCreation) {
    Bytecode *bc = new Bytecode();
    bc->push_back(new StoreIntConst(300, 7));
    bc->push_back(new StoreIntConst(301, 42));
    bc->push_back(new StoreConst(0, 301));
    bc->push_back(new Jmp(5));
    bc->push_back(new StoreName(0, "a"));
    bc->push_back(new End());

    BCBlob bcb(*bc, 0, bc->size());
    
    EXPECT_EQ(get_bcb_mnems(bcb), "STORE_INT_CONST\nSTORE_INT_CONST\nSTORE_CONST\nJMP\nSTORE_NAME\nEND\n");
    
    std::stringstream ss;
    for (unsigned i = 0; i < bcb.size(); ++i) {
        ss << bcb[i]->get_mnem() << "\n";
    }
    EXPECT_EQ(ss.str(), "STORE_INT_CONST\nSTORE_INT_CONST\nSTORE_CONST\nJMP\nSTORE_NAME\nEND\n");
    std::stringstream ss2;
    ss2 << bcb;
    EXPECT_EQ(ss2.str(), "; Blob\n0\tSTORE_INT_CONST  #300, 7\n1\tSTORE_INT_CONST  #301, 42\n2\tSTORE_CONST  %0,  #301\n3\tJMP  5\n4\tSTORE_NAME  %0, \"a\"\n5\tEND\n");

    BCBlob bcb2(*bc, 2, 5);
    EXPECT_EQ(get_bcb_mnems(bcb2), "STORE_CONST\nJMP\nSTORE_NAME\n");
    EXPECT_EQ(bcb2[0]->get_mnem(), "STORE_CONST");
    EXPECT_EQ(bcb2[1]->get_mnem(), "JMP");
    EXPECT_EQ(bcb2[2]->get_mnem(), "STORE_NAME");
    EXPECT_EQ(bcb2.front()->get_mnem(), "STORE_CONST");
    EXPECT_EQ(bcb2.front(), bcb2[0]);
    EXPECT_EQ(bcb2.back()->get_mnem(), "STORE_NAME");
    EXPECT_EQ(bcb2.back(), bcb2[bcb2.size()-1]);

    BCBlob bcb3(*bc, 5, 6);
    std::stringstream ss3;
    ss3 << bcb3;
    EXPECT_EQ(ss3.str(), "; Blob\n5\tEND\n");
    EXPECT_EQ(bcb3.size(), 1);

    BCBlob bcb4(*bc, 0, 0);
    std::stringstream ss4;
    ss4 << bcb4;
    EXPECT_EQ(ss4.str(), "; Blob\n");
    EXPECT_EQ(bcb4.size(), 0);

    EXPECT_TRUE(bcb4.isa_blob());
    EXPECT_FALSE(bcb4.isa_class());

    delete bc;
}

TEST(BytecodeBlob, BCParsing) {
    ustring code = R"(
d"Testing code"

space Space2Extend {
    fun foo() {
        fun inner(c) {
            class InnerClass {

            }
            return 4
        }
        return "new fun\n"
    }

    fun spcfun(o) = o + 1
}

fun goo(a) {
    a = 13
    fun o1(b) = b * $a
    return o1(a) 
}

space FooSpace {
    class Foo {
        fun Foo(a, b) {
            this.a = a
            this.b = b
        }

        fun __String() {
            return "Foo " ++ this.a ++ ", " ++ this.b
        }
    }
}

goo(4)
)";

    ustring expected = R"(
; ├─ Space blob [3; 32)
;   ├─ Function blob [5; 24)
;     └─ Function blob [9; 20)
;       └─ Class blob [13; 15)
;   └─ Function blob [24; 32)
; ├─ Function blob [33; 56)
;   └─ Function blob [40; 48)
; └─ Space blob [56; 89)
;   └─ Class blob [58; 88)
;     ├─ Function blob [60; 72)
;     └─ Function blob [72; 88)
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);
    cgen.generate(mod);
    BCBlob *bcb = BCBlob::parse_bc(*bc);

    std::stringstream ss;
    ss << "\n";
    bcb->print_bc_tree(ss);
    EXPECT_EQ(ss.str(), expected);

    delete bcb;
    delete bc;
    delete mod;
}

}