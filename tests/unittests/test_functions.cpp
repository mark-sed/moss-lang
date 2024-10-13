#include <gtest/gtest.h>
#include <initializer_list>
#include <algorithm>
#include "ir.hpp"
#include "testing_utils.hpp"

namespace{

using namespace moss;
using namespace ir;
using namespace testing;

ustring list_all(std::vector<ustring> names) {
    ustring l = "{";
    for (auto n: names)
        l += n+", ";
    l += "}";
    return l;
}

static void check_names(ustring name, 
                        std::vector<Argument *> args,
                        std::initializer_list<ustring> expected) {
    auto names = encode_fun(name, args);
    ASSERT_EQ(names.size(), expected.size()) << "Mismatched generated and expected names";
    for (auto e: expected) {
        auto exp = name + "(" + e + ")";
        EXPECT_TRUE(std::find(names.begin(), names.end(), exp) != names.end()) << "Expected name: '"
            << exp << "' was not generated\nAll generated names: " << list_all(names);
    }

    for(auto a: args) {
        delete a;
    }
}

static void check_names(Function *fun,
                        std::initializer_list<ustring> expected) {
    auto name = fun->get_name();
    auto names = encode_fun(fun);
    ASSERT_EQ(names.size(), expected.size()) << "Mismatched generated and expected names";
    for (auto e: expected) {
        auto exp = name + "(" + e + ")";
        EXPECT_TRUE(std::find(names.begin(), names.end(), exp) != names.end()) << "Expected name: '"
            << exp << "' was not generated\nAll generated names: " << list_all(names);
    }

    delete fun;
}

std::vector<Argument *> create_args(std::initializer_list<std::initializer_list<ustring>> types) {
    unsigned var_i = 0;
    std::vector<Argument *> args;

    for(auto i: types) {
        std::vector<Expression *> atps;
        for(auto j: i) {
            if (!j.empty())
                atps.push_back(new Variable(j));
        }
        auto name = std::string("v")+std::to_string(var_i++);
        args.push_back(new Argument(name, atps));
    }

    return args;
}

TEST(Functions, NameEncoding) {
    check_names(
        "foo", 
        std::vector<Argument *> {
            new Argument("a", std::vector<Expression *>{})
        },
        {"_"}
    );

    check_names(
        "baz", 
        std::vector<Argument *> {
            new Argument("a", std::vector<Expression *>{}),
            new Argument("b", std::vector<Expression *>{}),
            new Argument("c", std::vector<Expression *>{}),
            new Argument("d", std::vector<Expression *>{}),
        },
        {"_,_,_,_"}
    );

    check_names(
        "bar", 
        create_args({{""}, {"Int"}, {"Int", "Bool", "String"}}),
        {"_,Int,Int",
         "_,Int,Bool",
         "_,Int,String"}
    );

    check_names(
        "bar", 
        create_args({{"ClassC", "ClassB"}, {"Int", "Bool"}, {"Foo"}}),
        {"ClassC,Int,Foo",
         "ClassC,Bool,Foo",
         "ClassB,Int,Foo",
         "ClassB,Bool,Foo"}
    );

    check_names(
        "bar", 
        create_args({{"Int"}, {"Bool"}}),
        {"Int,Bool"}
    );

    check_names(
        "bar", 
        create_args({{"Foo::Goo", "Foo::Roo"}, {"Bool"}}),
        {"Foo::Goo,Bool",
         "Foo::Roo,Bool"}
    );

    {
        // (arg1:[SomeSpace::SomeValue, MyBool])
        std::vector<Argument *> args {
            new Argument("arg1", std::vector<Expression *> {
                new BinaryExpr(
                    new Variable("SomeSpace"),
                    new Variable("SomeValue"),
                    Operator(OperatorKind::OP_SCOPE)
                ),
                new Variable("MyBool")}
            )
        };
        Function *fun = new Function("some_function123", args, std::list<IR *>{});
        check_names(
            fun,
            {"SomeSpace::SomeValue",
             "MyBool"}
        );
    }

    {
        // (arg1, arg2:SomeSpace::SomeOtherSpace::SomeValue, arg3:[String,String2])
        std::vector<Argument *> args {
            new Argument("arg1", std::vector<Expression *>{}),
            new Argument("arg2", std::vector<Expression *> {
                new BinaryExpr(
                    new Variable("SomeSpace"),
                    new BinaryExpr(
                        new Variable("SomeOtherSpace"),
                        new Variable("SomeValue"),
                        Operator(OperatorKind::OP_SCOPE)
                    ),
                    Operator(OperatorKind::OP_SCOPE)
                )
            }),
            new Argument("arg3", std::vector<Expression *>{
                new Variable("String"),
                new Variable("String2")}
            )
        };
        Function *fun = new Function("fun2", args, std::list<IR *>{});
        check_names(
            fun,
            {"_,SomeSpace::SomeOtherSpace::SomeValue,String",
             "_,SomeSpace::SomeOtherSpace::SomeValue,String2"}
        );
    }
}

}