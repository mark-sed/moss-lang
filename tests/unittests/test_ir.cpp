#include <gtest/gtest.h>
#include "ir.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"
#include <sstream>
#include <string>
#include <regex>

namespace{

using namespace moss;
using namespace testing;

/** Test correct casting */
TEST(IR, Casting){
    SourceInfo srci;
    ir::Module *m = new ir::Module("m", srci);
    ir::IR *i = m;

    EXPECT_TRUE(isa<ir::Module>(*i));
    EXPECT_TRUE(isa<ir::Module>(i));
    EXPECT_TRUE(isa<ir::Module>(m));
    EXPECT_TRUE(dyn_cast<ir::Module>(i));

    EXPECT_FALSE(isa<ir::Space>(i));
    EXPECT_FALSE(isa<ir::Space>(m));
    EXPECT_FALSE(dyn_cast<ir::Space>(i));

    delete m;
}

/** Check that anonymous objects have unique names for lookup */
TEST(IR, AnonymousSpaceNames){
    ustring code = R"(
space {

}

space SomeSpace {

}

space {
    space {}
}

space {}
)";

    /*
    // The actual numbers might differ as the counter is static and some other
    // tests might have incremented it so we cannot check for the exact number
    <Module>test{
        space 0s { }
        space SomeSpace { }
        space 2s {
            space 1s { }
        }
        space 3s { }
        <IR: <end-of-file>>
    }
    */

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    // Convert list of IRs into vector for easy access
    std::list<ir::IR *> body_list = mod->get_body();
    std::vector<ir::IR *> body{ std::begin(body_list), std::end(body_list) };

    ir::Space *sp1 = dyn_cast<ir::Space>(body[0]);
    ASSERT_TRUE(sp1);
    ir::Space *sp2 = dyn_cast<ir::Space>(body[1]);
    ASSERT_TRUE(sp2);
    ir::Space *sp3 = dyn_cast<ir::Space>(body[2]);
    ASSERT_TRUE(sp3);
    ir::Space *sp4 = dyn_cast<ir::Space>(body[3]);
    ASSERT_TRUE(sp4);
    
    EXPECT_TRUE(sp1 != sp3);
    EXPECT_TRUE(sp1 != sp4);
    EXPECT_TRUE(sp3 != sp4);

    auto an_space_name_rx = std::regex("[0-9]+s");
    EXPECT_TRUE(std::regex_match(sp1->get_name(), an_space_name_rx));
    EXPECT_EQ(sp2->get_name(), "SomeSpace");
    // There is one more space inside of the one after SomeSpace
    // and since it is parsed fully first, it takes 1s
    EXPECT_TRUE(std::regex_match(sp3->get_name(), an_space_name_rx));
    EXPECT_TRUE(std::regex_match(sp4->get_name(), an_space_name_rx));


    delete mod;
}

/** Check that anonymous lambdas have unique names for lookup */
TEST(IR, AnonymousLambdaNames){
    ustring code = R"(
fun(x) = x
fun foo() = "ji"
fun(a, b) = 3
fun(g) = true
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    // Convert list of IRs into vector for easy access
    std::list<ir::IR *> body_list = mod->get_body();
    std::vector<ir::IR *> body{ std::begin(body_list), std::end(body_list) };

    ir::Lambda *l1 = dyn_cast<ir::Lambda>(body[0]);
    ASSERT_TRUE(l1);
    ir::Lambda *l2 = dyn_cast<ir::Lambda>(body[1]);
    ASSERT_TRUE(l2);
    ir::Lambda *l3 = dyn_cast<ir::Lambda>(body[2]);
    ASSERT_TRUE(l3);
    ir::Lambda *l4 = dyn_cast<ir::Lambda>(body[3]);
    ASSERT_TRUE(l4);
    
    EXPECT_TRUE(l1 != l3);
    EXPECT_TRUE(l1 != l4);
    EXPECT_TRUE(l3 != l4);

    EXPECT_EQ(l1->get_name(), "0l");
    EXPECT_EQ(l2->get_name(), "foo");
    EXPECT_EQ(l3->get_name(), "1l");
    EXPECT_EQ(l4->get_name(), "2l");

    delete mod;
}

/** Check correct parent setup in IR */
TEST(IR, IRParents){
    ustring code = R"(
class Cls {
    fun foo(a, b) {
        c = a + 1
        if (b)
            c += 2
        return c
    }

    space Sp1 {
        a = 4
    }
}

if (true) {
    "hello"
} else {
    "bye"
}
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    EXPECT_FALSE(mod->get_parent());

    // Convert list of IRs into vector for easy access
    std::list<ir::IR *> body_list = mod->get_body();
    std::vector<ir::IR *> body{ std::begin(body_list), std::end(body_list) };

    auto cls = dyn_cast<ir::Class>(body[0]);
    EXPECT_EQ(cls->get_parent(), nullptr);

    auto body_cls_list = cls->get_body();
    std::vector<ir::IR *> body_cls{ std::begin(body_cls_list), std::end(body_cls_list) };
    auto foo = dyn_cast<ir::Function>(body_cls[0]);
    EXPECT_EQ(foo->get_parent(), cls); // foo
    auto body_foo_list = foo->get_body();

    EXPECT_EQ(foo->get_args()[0]->get_parent(), foo);
    std::vector<ir::IR *> body_foo{ std::begin(body_foo_list), std::end(body_foo_list) };
    auto c = dyn_cast<ir::BinaryExpr>(body_foo[0]);
    EXPECT_EQ(c->get_parent(), foo);
    EXPECT_EQ(c->get_left()->get_parent(), c);
    EXPECT_EQ(c->get_right()->get_parent(), c);

    auto ifb = dyn_cast<ir::If>(body_foo[1]);
    EXPECT_EQ(ifb->get_parent(), foo);
    EXPECT_EQ(ifb->get_cond()->get_parent(), ifb);
    auto ifc = dyn_cast<ir::BinaryExpr>(ifb->get_body().back());
    EXPECT_EQ(ifc->get_parent(), ifb);
    EXPECT_EQ(ifc->get_left()->get_parent(), ifc);
    EXPECT_EQ(ifc->get_right()->get_parent(), ifc);

    auto ret = dyn_cast<ir::Return>(body_foo[2]);
    EXPECT_EQ(ret->get_parent(), foo);
    EXPECT_EQ(ret->get_expr()->get_parent(), ret);

    auto sp1 = dyn_cast<ir::Space>(body_cls[1]);
    EXPECT_EQ(sp1->get_parent(), cls); // space Sp1
    EXPECT_EQ(sp1->get_body().back()->get_parent(), sp1);

    auto i = dyn_cast<ir::If>(body[1]);
    EXPECT_EQ(i->get_parent(), nullptr);
    EXPECT_EQ(i->get_body().back()->get_parent(), i);

    auto e = i->get_else();
    EXPECT_EQ(e->get_parent(), nullptr);
    EXPECT_EQ(e->get_body().back()->get_parent(), e);

    delete mod;
}

}