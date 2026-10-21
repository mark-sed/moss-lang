#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "analysis/expression_analyzer.hpp"
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

/// This tests that there cannot be break and continue outside of a loop.
TEST(ExpressionAnalysis, BreakAndContinueOutsideOfLoop){
    std::vector<ustring> lines = {
"break",
"continue",
"space F { break; }",
"class X { fun X() { continue; }; }",
};

    testing::check_all_lines_err(lines, "BreakAndContinueOutsideOfLoop");

    std::vector<ustring> lines2 = {
"for (x: [1,2,3]) break",
"do { continue; } while(false)",
"while (true) { break; }",
};

    testing::check_all_lines_ok(lines2, "BreakAndContinueOutsideOfLoop");
}

/// This tests path imports and space imports
TEST(ExpressionAnalysis, ScopePathImports){
    std::vector<ustring> lines = {
"import ::\"Module.ms\"",
"import ::\"Module.ms\".Mod",
"import a, ::\"hello\"",
"import a, ::\"hello\".h",
"import ::\"A\".A",
"import ::\"C1\" as c1",
"import ::\"C1\".c1 as c1",
"import c, b as b1, ::\"cc2\".c2 as ccc2, h",
};

    testing::check_all_lines_err(lines, "ScopePathImports");

    std::vector<ustring> lines2 = {
"import ::A.h",
"import a, ::B.c",
"import ::C.f as f",
"import ::C.c.d as d1",
"import a2 as a22, ::b.b as bb, ::c.c",
};

    testing::check_all_lines_ok(lines2, "ScopePathImports");
}

}