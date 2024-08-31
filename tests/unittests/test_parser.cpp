#include <gtest/gtest.h>
#include "parser.hpp"
#include "source.hpp"
#include "ir.hpp"
#include "os_interface.hpp"
#include <sstream>
#include <string>

namespace{

using namespace moss;
using namespace ir;

/** Test correct parsing regardless of the new line */
TEST(Parsing, WhiteSpace){
    ustring code = R"(


// Comment

  /* Another comment


  */ // And here as well


// Comment
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    ASSERT_TRUE(mod->size() == 1) << "Empty program has more/less than just EOF";
    EXPECT_TRUE(isa<EndOfFile>(mod->back()));

    delete mod;
}

TEST(Parsing, Statements){
    ustring code = R"(
// Assert
assert(a, b);
assert(cond)

// Raise
raise except

// Return
return
return value

// Continue
continue

// Break
break
)";

    IRType expected[] = {
        IRType::ASSERT,
        IRType::ASSERT,
        
        IRType::RAISE,

        IRType::RETURN,
        IRType::RETURN,

        IRType::CONTINUE,
        IRType::BREAK,
        
        IRType::END_OF_FILE
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index;
    }

    delete mod;
}

TEST(Parsing, SimpleValues){
    ustring code = R"(
4545156
-121
0.054
-0.0
true
false
not true
-----9 // Still valid
not not not false
nil
)";

    IRType expected[] = {
        IRType::INT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::FLOAT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::BOOL_LITERAL,
        IRType::BOOL_LITERAL,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::NIL_LITERAL,
        
        IRType::END_OF_FILE
    };

    OperatorKind expected_ops[] = {
        OperatorKind::OP_NEG,
        OperatorKind::OP_NEG,
        OperatorKind::OP_NOT,
        OperatorKind::OP_NEG,
        OperatorKind::OP_NOT,
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    int op_index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index;
        if (decl->get_type() == IRType::UNARY_EXPR) {
            auto un_expr = dyn_cast<UnaryExpr>(decl);
            EXPECT_TRUE(un_expr->get_op().get_kind() == expected_ops[op_index++]) << "Incorrect Operator at index: " 
                << index << " (" << op_index << ")" << "\nExpected: " << Operator(expected_ops[op_index-1]) << "\nBut got: " << un_expr->get_op();
        }
        else if (decl->get_type() == IRType::BINARY_EXPR) {
            
        }
    }

    delete mod;
}

TEST(Parsing, Expressions){
    ustring code = R"(-a > 4
-(a > 9 < -8 <= 8)
not -a > -4
----+a
a >= a and b
a >= (not a) // Parenthesis are needed
not a > 4

true and false and true or a xor b and not c

a && b > 4 || (c and b && -f)
//false || ~a

(a > 5 ? -4 : "hello")
(a > 4 ? c : d) != 0 ? some : not other

value in something
not 4 in somewhere
"x" in "hello " ++ "x" ++ "y"

a == b
b == c != d == (not g)

_1 = _2 = _3 = "hello"

a + 4 + b * -2
a * b / x % 3 - -1
((a + b) * (2 / v) + 1)
a ^ b ^ -1
b ^ (4 * 2 ^ a ^ 2)

a[-2]
g2["sfd"][a+1][8]
a[1+a^-2]

"Hello"[1,2..length]
a[-2*a,-2*a-2..some_value - 10]

foo()
foo(true)
foo(a+2, a++4, 1+(1*2))
foo(1,3..4) // 1 has precedence as an argument

std::math::pi + 4
foo::goo
)";

    ustring expected = R"(((- a) > 4)
(- (((a > 9) < (- 8)) <= 8))
(not ((- a) > (- 4)))
(- (- (- (- a))))
((a >= a) and b)
(a >= (not a))
(not (a > 4))
(((((true and false) and true) or a) xor b) and (not c))
((a && (b > 4)) || ((c and b) && (- f)))
((a > 5) ? (- 4) : "hello")
((((a > 4) ? c : d) != 0) ? some : (not other))
(value in something)
(not (4 in somewhere))
("x" in (("hello " ++ "x") ++ "y"))
(a == b)
(((b == c) != d) == (not g))
(_1 = (_2 = (_3 = "hello")))
((a + 4) + (b * (- 2)))
((((a * b) / x) % 3) - (- 1))
(((a + b) * (2 / v)) + 1)
(a ^ (b ^ (- 1)))
(b ^ (4 * (2 ^ (a ^ 2))))
(a [] (- 2))
(((g2 [] "sfd") [] (a + 1)) [] 8)
(a [] (1 + (a ^ (- 2))))
("Hello" [] (1, 2..length))
(a [] (((- 2) * a), (((- 2) * a) - 2)..(some_value - 10)))
foo()
foo(true)
foo((a + 2), (a ++ 4), (1 + (1 * 2)))
foo(1, (3..4))
(((std :: math) :: pi) + 4)
(foo :: goo)
<IR: <end-of-file>>
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    std::stringstream ss;
    for (auto decl: mod->get_body()) {
        ss << *decl << "\n";
    }

    EXPECT_EQ(ss.str(), expected);

    delete mod;
}

}