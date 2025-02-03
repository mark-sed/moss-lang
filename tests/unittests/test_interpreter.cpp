#include <gtest/gtest.h>
#include "ir.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"
#include "interpreter.hpp"
#include "bytecodegen.hpp"
#include <sstream>
#include <string>

using namespace moss;
using namespace testing;

/** Test to make sure that object and class has access only to its attributes */
TEST(Interpreter, Attributes){
    ustring code = R"(
@foo_annot
class M {
    M_VAR = 4

    fun M(a1) {
        this.name = a1
    }

    fun me() {}
}

m = M("smol m")
m.v = true
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);
    cgen.generate(mod);

    Interpreter *i = new Interpreter(bc, nullptr, true);
    i->run();

    EXPECT_EQ(i->get_exit_code(), 0);

    auto m = i->load_name("m");
    ASSERT_TRUE(m);

    auto M = i->load_name("M");
    ASSERT_TRUE(M);

    ASSERT_TRUE(m->get_attrs());
    ASSERT_TRUE(M->get_attrs());

    // m
    EXPECT_TRUE(m->has_attr("name"));
    EXPECT_TRUE(m->has_attr("v"));
    EXPECT_TRUE(m->has_attr("M_VAR"));
    EXPECT_TRUE(m->has_attr("me"));
    // Object can see its constructor
    EXPECT_TRUE(m->has_attr("M"));
    EXPECT_FALSE(m->has_attr("this"));

    EXPECT_FALSE(m->has_annotation("foo_annot"));

    // M
    EXPECT_TRUE(M->has_attr("M_VAR"));
    EXPECT_TRUE(M->has_attr("me"));
    EXPECT_TRUE(M->has_attr("M"));
    EXPECT_FALSE(M->has_attr("name"));
    EXPECT_FALSE(M->has_attr("v"));
    EXPECT_FALSE(M->has_attr("a1"));
    EXPECT_FALSE(M->has_attr("this"));

    EXPECT_TRUE(M->has_annotation("foo_annot"));

    delete i;
    delete bc;
    delete mod;
}