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

/// This tests that ExpressionAnalysis reports incorrect member access.
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

    testing::check_all_lines_err(lines, "MemberAccess");

    ustring correct = R"(
1 + (2).foo();
a.b.c.f(3)
)";

    testing::check_line_ok(correct, "MemberAccess");
}

/// This tests that ExpressionAnalysis reports incorrect argument expressions.
TEST(ExpressionAnalysis, ArgAnalysis){
    std::vector<ustring> lines = {
"foo(a.v=3, 5)",
"fun ff(v, a) {}; ff(a=4, a=4)",
"fun som(a, b, c) {}; som(a=3, b=3, a=5)"
};

    testing::check_all_lines_err(lines, "ArgAnalysis");
}

}