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

TEST(Parsing, AllExpressionTypes){
    ustring code = R"(
4545156
-121
0.054
-0.0
true
false
"moss language"
not true
-----9 // Still valid
not not not false
(nil)
hello
a ? true : nil
1,4..someval
foo(1, 2)
foo(1, (1,3..9))
note"Some value"
md"""
# Title
"""

a ++ b ++ "\n"
2 ^ b ^ 2
4 + g1 + c2
8 - g2 - 2 - a
h / 2 / a
g * 2 * g
a % 3
a = b = 3
a ++= "!"
b ^= 2 * a
c += (a)
g -= a
hu /= a ? 2 : 4
hy *= 5
jh %= 2
~a
c == a == 5
c != 45
a > 4 > b
c < 3
g >= 2*r
g <= 22
a && foo()
a || goo()
true and false
false or c
42 xor a xor c
"a" in "hi"
a.access.b
some[2]
some[1,2..60]
std::math::pi
::NAME
)";

    IRType expected[] = {
        IRType::INT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::FLOAT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::BOOL_LITERAL,
        IRType::BOOL_LITERAL,
        IRType::STRING_LITERAL,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::NIL_LITERAL,
        IRType::VARIABLE,
        IRType::TERNARY_IF,
        IRType::RANGE,
        IRType::CALL,
        IRType::CALL,
        IRType::NOTE,
        IRType::NOTE,

        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::BINARY_EXPR,
        IRType::UNARY_EXPR,
        
        IRType::END_OF_FILE
    };

    OperatorKind expected_ops[] = {
        OperatorKind::OP_MINUS,
        OperatorKind::OP_MINUS,
        OperatorKind::OP_NOT,
        OperatorKind::OP_MINUS,
        OperatorKind::OP_NOT,

        
        OperatorKind::OP_CONCAT,
        OperatorKind::OP_EXP,
        OperatorKind::OP_PLUS,
        OperatorKind::OP_MINUS,
        OperatorKind::OP_DIV,
        OperatorKind::OP_MUL,
        OperatorKind::OP_MOD,
        OperatorKind::OP_SET,        
        OperatorKind::OP_SET_CONCAT, 
        OperatorKind::OP_SET_EXP,    
        OperatorKind::OP_SET_PLUS,   
        OperatorKind::OP_SET_MINUS,  
        OperatorKind::OP_SET_DIV,    
        OperatorKind::OP_SET_MUL,    
        OperatorKind::OP_SET_MOD,    
        OperatorKind::OP_SILENT,
        OperatorKind::OP_EQ,  
        OperatorKind::OP_NEQ, 
        OperatorKind::OP_BT,  
        OperatorKind::OP_LT,  
        OperatorKind::OP_BEQ, 
        OperatorKind::OP_LEQ, 
        OperatorKind::OP_SHORT_C_AND,
        OperatorKind::OP_SHORT_C_OR, 
        OperatorKind::OP_AND,   
        OperatorKind::OP_OR,
        OperatorKind::OP_XOR,   
        OperatorKind::OP_IN,
        OperatorKind::OP_ACCESS,
        OperatorKind::OP_SUBSC,
        OperatorKind::OP_SUBSC,
        OperatorKind::OP_SCOPE,
        OperatorKind::OP_SCOPE,
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
                << index << " (" << op_index-1 << ")" << "\nExpected: " << Operator(expected_ops[op_index-1]) << "\nBut got: " << un_expr->get_op();
        }
        else if (decl->get_type() == IRType::BINARY_EXPR) {
            auto bin_expr = dyn_cast<BinaryExpr>(decl);
            EXPECT_TRUE(bin_expr->get_op().get_kind() == expected_ops[op_index++]) << "Incorrect Operator at index: " 
                << index << " (" << op_index-1 << ")" << "\nExpected: " << Operator(expected_ops[op_index-1]) << "\nBut got: " << bin_expr->get_op();
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

TEST(Parsing, Spaces){
    ustring code = R"(
space foo { Some_val = 4; goo(); space foo2 {} } a = 4

space {
    space {
        
        space {space{}}
    }
}

space o1_1231
{

    1 + 1

}

space toto {

    space rosana {

    }

    space gift {
        space of {
            space faith {}
        }
    }
}
)";

    IRType expected[] = {
        IRType::SPACE,
        IRType::BINARY_EXPR,
        IRType::SPACE,
        IRType::SPACE,
        IRType::SPACE,
        
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

TEST(Parsing, Enums){
    ustring code = R"(
enum Foo {}
enum Foo2 
{


}

enum Bar {
    START
}
enum Bar2 {START}

enum Baz {
    START;
    END; MID, MAX
    LEFT
}

enum Colors { BLUE, RED, WHITE, GRAY, }
)";

    IRType expected[] = {
        IRType::ENUM,
        IRType::ENUM,
        IRType::ENUM,
        IRType::ENUM,
        IRType::ENUM,
        IRType::ENUM,

        IRType::END_OF_FILE
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index-1;
    }

    delete mod;

    // Errors
ustring incorrect = R"(
enum
enum {}
enum Foo2 {a b}
enum Foo3 { hi, 2 }
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    SourceFile sf2(incorrect, SourceFile::SourceType::STRING);
    Parser parser2(sf2);

    bool eof_reached = false;
    index = 0;
    while (!eof_reached) {
        std::vector<ir::IR *> line_irs = parser2.parse_line();
        for (auto i : line_irs) {
            EXPECT_TRUE(i->get_type() == expected_incorr[index++]) << "Incorrect IR at index: " << index-1;
            if (isa<ir::EndOfFile>(i)) {
                eof_reached = true;
            }
        }

        for (auto i : line_irs) {
            delete i;
        }
    }
}

}