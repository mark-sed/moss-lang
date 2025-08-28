#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "function_analyzer.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace testing;

/// This tests that FunctionAnalysis reports duplicate argument names.
TEST(FunctionAnalysis, DuplicateArgs){
    std::vector<ustring> lines = {
"fun g(a, b, a, c) {}",
"fun g(a, b, ... a) {}",
"class Cls { fun Cls(a, v, g, a) {}; }",
"class Cls { fun a(a, v, g, a) {}; }",
"space f { fun f(a, a) {}; }",
"fun d(a:[Bool,String], b, a:Int) {}",
"fun(a, b, a=4) = 4",
"class C { fun foo(a, b, b)=nil; }",
"space s { fun(a, v, v)=true; }",
"lam = fun lambda(a, no, b, no) = 45",
};

    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod);
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_TRUE(err);
        auto rs = dyn_cast<ir::Raise>(err);
        ASSERT_TRUE(rs);

        delete mod;
        delete err;
    }
}

/// Checks that args after varag have default value
TEST(FunctionAnalysis, NonDefaultArgAfterVarargs){
    std::vector<ustring> lines = {
"fun g(a, b, ... t, c) {}",
"class F { fun g(a, b, ... t, c) {}; }",
"class F { fun F(a, ...b, f) {}; }",
"space S { fun foo(a, l, ...m, o) {}; }",
"fun(a, ...b, t=5, c) = 43",
"class F { fun foo(a, b=4, ...c, t) = 4; }",
"space { fun(a, b, ...d, o, p) = false; }",
};

    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod);
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_TRUE(err);
        auto rs = dyn_cast<ir::Raise>(err);
        ASSERT_TRUE(rs);

        delete mod;
        delete err;
    }
}

/// Checks that args after default value have default value
TEST(FunctionAnalysis, NonDefaultArgAfterDefault){
    std::vector<ustring> lines = {
"fun foo(a=4, b) {}",
"fun goo(a, b, c:[Int,Float]=5, d) {}",
"fun(a=4, b)=a",
"class C { fun C(a, b=4, f) {}; }",
"class C { fun f(a=4, b){}; }",
"class Doe { fun i(ano=4, ne) = ne; }",
"space DF { fun foo(a, b=4, c, d=4) {}; }",
};

    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod);
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_TRUE(err);
        auto rs = dyn_cast<ir::Raise>(err);
        ASSERT_TRUE(rs);

        delete mod;
        delete err;
    }
}


}