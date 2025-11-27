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

    testing::check_all_lines_err(lines, "DuplicateArgs");
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

    testing::check_all_lines_err(lines, "NonDefaultArgAfterVarargs");
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

    testing::check_all_lines_err(lines, "NonDefaultArgAfterDefault");
}

/// This tests that FunctionAnalysis reports when operator function is declared
/// outside of a class.
TEST(FunctionAnalysis, OperatorFunsOutsideOfClass){
    std::vector<ustring> lines = {
"fun (+)(a) { return this.x + a + this.off; }",
"fun (+)(a) = this.x + a + this.off;",
"fun (-)(a) {return this.x - a + this.off;}",
"fun (-)() {return -this.x + this.off;}",
"fun (==)(a:NumOffset) { return this.x == a.x and this.off == a.off;}",
"fun (!=)(a:NumOffset) {return not this.(==)(a);}",
"fun (/)(a) { return this.x / a + this.off; }",
"fun (*)(a) { return this.x * a + this.off; }",
"fun (^)(a) { return (this.x ^ a) + this.off; }",
"fun (%)(a) { return this.x % a + this.off;}",
"fun (>)(a) { return this.x + this.off > a; }",
"fun (<)(a) {  return this.x + this.off < a;}",
"fun (>=)(a) { return this.x + this.off >= a;}",
"fun (<=)(a) { return this.x + this.off <= a;}",
"fun (in)(a) {return a <= this.x + this.off;}",
"fun (and)(a) {return a and (this.x + this.off);}",
"fun (or)(a) = a or (this.x + this.off)",
"fun (xor)(a) {return a xor (this.x + this.off);}",
"fun (not)() {return not (this.x + this.off); }",
"fun (())(a:Int, b) {return this.x + a + b;}",
"fun (())() {return nil;}",
"fun ([])(other) = other",
};
    testing::check_all_lines_err(lines, "OperatorFunsOutsideOfClass");
}

/// Tests for incorrect annotations or function signatures based on annotations.
TEST(FunctionAnalysis, MismatchedFunsAndAnnotations){
    std::vector<ustring> lines = {
R"(fun pt2pta(a) { @!generator("pt"); @!converter("txt", "pt"); })",
R"(@generator("pt") @converter("txt", "pt") fun pt2pta(a) = a)",
R"(@converter("pt", "pta") fun pt2pta(a, b) {})",
R"(@converter("pt", "pta") fun pt2pta(a, b) = a)",
R"(@generator("pt") fun pt2pta2() { a; })",
};
    testing::check_all_lines_err(lines, "MismatchedFunsAndAnnotations");
}

/// Tests for returns outside of functions
TEST(FunctionAnalysis, ReturnsOutsideOfFunctions){
    std::vector<ustring> lines = {
R"(return 43)",
R"(space Sp { a = 4; return a; })",
R"(class A { return 4; })",
};
    testing::check_all_lines_err(lines, "ReturnsOutsideOfFunctions");
}

}