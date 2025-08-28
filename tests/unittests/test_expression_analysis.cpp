#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "expression_analyzer.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace testing;

/// This tests that FunctionAnalysis reports duplicate argument names.
TEST(ExpressionAnalysis, MemberAccess){
    // Cannot test .* as that is checked in parser not analyzer.
    std::vector<ustring> lines = {
"1 * a.4",
"x.b.(2)",
"a.\"hello\"",
"og.$g",
"a.::b",
//"::$a.boo",
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

    ustring correct = R"(
1 + (2).foo();
a.b.c.f(3)
)";

    SourceFile sf(correct, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ASSERT_TRUE(mod);
    ir::IRPipeline irp(parser);
    auto err = irp.run(mod);
    ASSERT_FALSE(err) << "Failed but was supposed to pass";

    delete mod;
    delete err;
}

}