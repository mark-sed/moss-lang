#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "method_analyzer.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace testing;

void check_line_err(ustring code, ustring test) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ASSERT_TRUE(mod) << test << ": " << code;
    ir::IRPipeline irp(parser);
    auto err = irp.run(mod);
    ASSERT_TRUE(err) << test << ": " << code;
    auto rs = dyn_cast<ir::Raise>(err);
    ASSERT_TRUE(rs) << test << ": " << code;

    delete mod;
    delete err;
}

/// This tests that MethodAnalysis correctly sets the constructor and method
/// tags in the Function and Lambda IR
TEST(MethodAnalysis, ConstructorAndMethodTagging){
    ustring code = R"(
fun foo1(a) {}
fun foo2() = 4

class Cl1 {
    fun Cl1(a) {}
    fun Cl1(a, b) {}

    fun get_f() = "42"
    fun get_g() {return 4;}

    class Cl2 {
        fun Cl2() {}
        fun Cl1() {}

        fun f1() {}
    }
}

space Spc1 {
    class Cl3 {
        fun f2() {}
        space {
            fun f3() {}
        }
        fun Cl3(a, b:Int) {}
    }
    fun f4(a, b, ... c) {}
}

fun foo3() {}
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ir::IRPipeline irp(parser);
    auto err = irp.run(mod);
    ASSERT_FALSE(err);

    // Convert list of IRs into vector for easy access
    auto body = list2vect(mod->get_body());

    ir::Function *foo1 = dyn_cast<ir::Function>(body[0]);
    ASSERT_TRUE(foo1);
    EXPECT_FALSE(foo1->is_method());
    EXPECT_FALSE(foo1->is_constructor());

    ir::Lambda *foo2 = dyn_cast<ir::Lambda>(body[1]);
    ASSERT_TRUE(foo2);
    EXPECT_FALSE(foo2->is_method());

    ir::Class *cl1 = dyn_cast<ir::Class>(body[2]);
    ASSERT_TRUE(cl1);
    auto cl1_body = list2vect(cl1->get_body());
    
    auto cl1_cons = dyn_cast<ir::Function>(cl1_body[0]);
    ASSERT_TRUE(cl1_cons);
    EXPECT_TRUE(cl1_cons->is_constructor());
    EXPECT_TRUE(cl1_cons->is_method());

    auto cl1_cons2 = dyn_cast<ir::Function>(cl1_body[1]);
    ASSERT_TRUE(cl1_cons2);
    EXPECT_TRUE(cl1_cons2->is_constructor());
    EXPECT_TRUE(cl1_cons2->is_method());

    auto get_f = dyn_cast<ir::Lambda>(cl1_body[2]);
    ASSERT_TRUE(get_f);
    EXPECT_TRUE(get_f->is_method());

    auto get_g = dyn_cast<ir::Function>(cl1_body[3]);
    ASSERT_TRUE(get_g);
    EXPECT_FALSE(get_g->is_constructor());
    EXPECT_TRUE(get_g->is_method());

    ir::Class *cl2 = dyn_cast<ir::Class>(cl1_body[4]);
    ASSERT_TRUE(cl2);
    auto cl2_body = list2vect(cl2->get_body());

    auto cl2_cons = dyn_cast<ir::Function>(cl2_body[0]);
    ASSERT_TRUE(cl2_cons);
    EXPECT_TRUE(cl2_cons->is_constructor());
    EXPECT_TRUE(cl2_cons->is_method());

    auto cl2_cl1 = dyn_cast<ir::Function>(cl2_body[1]);
    ASSERT_TRUE(cl2_cl1);
    EXPECT_FALSE(cl2_cl1->is_constructor());
    EXPECT_TRUE(cl2_cl1->is_method());

    auto f1 = dyn_cast<ir::Function>(cl2_body[2]);
    ASSERT_TRUE(f1);
    EXPECT_FALSE(f1->is_constructor());
    EXPECT_TRUE(f1->is_method());

    ir::Space *spc1 = dyn_cast<ir::Space>(body[3]);
    ASSERT_TRUE(spc1);
    auto spc1_body = list2vect(spc1->get_body());

    ir::Class *cl3 = dyn_cast<ir::Class>(spc1_body[0]);
    ASSERT_TRUE(cl3);
    auto cl3_body = list2vect(cl3->get_body());

    auto f2 = dyn_cast<ir::Function>(cl3_body[0]);
    ASSERT_TRUE(f2);
    EXPECT_FALSE(f2->is_constructor());
    EXPECT_TRUE(f2->is_method());

    ir::Space *spc1_spc = dyn_cast<ir::Space>(cl3_body[1]);
    ASSERT_TRUE(spc1);
    auto spc1_spc_body = list2vect(spc1_spc->get_body());

    auto f3 = dyn_cast<ir::Function>(spc1_spc_body[0]);
    ASSERT_TRUE(f3);
    EXPECT_FALSE(f3->is_constructor());
    EXPECT_FALSE(f3->is_method());

    auto cl3_constr = dyn_cast<ir::Function>(cl3_body[2]);
    ASSERT_TRUE(cl3_constr);
    EXPECT_TRUE(cl3_constr->is_constructor());
    EXPECT_TRUE(cl3_constr->is_method());

    auto f4 = dyn_cast<ir::Function>(spc1_body[1]);
    ASSERT_TRUE(f4);
    EXPECT_FALSE(f4->is_constructor());
    EXPECT_FALSE(f4->is_method());

    ir::Function *foo3 = dyn_cast<ir::Function>(body[4]);
    ASSERT_TRUE(foo3);
    EXPECT_FALSE(foo3->is_method());
    EXPECT_FALSE(foo3->is_constructor());

    delete mod;
}

/// This tests that MethodAnalysis raises error when lambda is used as a constructor
TEST(MethodAnalysis, LambdaConstructors){
    ustring code = R"(
class SomeClass {
    fun SomeClass() = nil
}
)";

    check_line_err(code, "LambdaConstructors");
}

/// This tests that MethodAnalysis raises error there is non-nil return in a constructor
TEST(MethodAnalysis, NonNilReturnInConstructor){
    ustring code = R"(
class SomeClass {
    fun SomeClass(x) {
        if (x == 4)
            return
        else
            return "hi"
    }
}
)";

    check_line_err(code, "NonNilReturnInConstructor");
}

/// Test reporting of incorrect annotations over methods.
TEST(MethodAnalysis, DisallowedAnnotations){
    ustring code = R"(
class X {
    @generator("txt")
    fun foo(x) {

    }
}
)";

    check_line_err(code, "DisallowedAnnotations");

    ustring code2 = R"(
class X {
    @converter("txt", "hi")
    fun foo(x) {

    }
}
)";

    check_line_err(code2, "DisallowedAnnotations");
}

}