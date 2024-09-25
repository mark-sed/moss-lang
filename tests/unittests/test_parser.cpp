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

// Annotations
@!SomeIDAnnotation
@!min_version("1.1")
)";

    IRType expected[] = {
        IRType::ASSERT,
        IRType::ASSERT,
        
        IRType::RAISE,

        IRType::RETURN,
        IRType::RETURN,

        IRType::CONTINUE,
        IRType::BREAK,

        IRType::ANNOTATION,
        IRType::ANNOTATION,
        
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

static void run_parser(ustring code, IRType *expected, unsigned expected_size) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    unsigned index = 0;
    ASSERT_EQ(expected_size, mod->size());
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index-1
            << "\nExpected: " << static_cast<unsigned>(expected[index-1]) << "\nBut got: " 
            << static_cast<unsigned>(decl->get_type()) << " - " << decl->get_name();
    }

    delete mod;
}

static void run_parser_by_line(ustring code, IRType *expected, unsigned expected_size) {
    ASSERT_TRUE(expected_size > 0) << "By line parsing expects more than 0 IRs";
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    bool eof_reached = false;
    unsigned index = 0;
    while (!eof_reached) {
        ASSERT_TRUE(index < expected_size) << " index: " << index << ", expected_size: " << expected_size;
        std::vector<ir::IR *> line_irs = parser.parse_line();
        ASSERT_TRUE(line_irs.size() <= 1);
        if (!line_irs.empty()) {
            EXPECT_TRUE(line_irs[0]->get_type() == expected[index++]) << "Incorrect IR at index: " << index-1
                << "\nExpected: " << static_cast<unsigned>(expected[index-1]) << "\nBut got: " 
                << static_cast<unsigned>(line_irs[0]->get_type()) << " - " << line_irs[0]->get_name();
            if (isa<ir::EndOfFile>(line_irs[0])) {
                eof_reached = true;
            }
        }

        for (auto i : line_irs) {
            delete i;
        }
    }
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

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(

space

space Foo

space Goo::Foo {}

space Foo+1

)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
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

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
enum
enum {}
enum Foo2 {a b}
enum Foo3 { 
hi,
Hi2 
2 
}
enum Foo4 {
A
B
A
}
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        
        IRType::RAISE,
        IRType::RAISE,

        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, Ifs){
    ustring code = R"(
if (true) "hi"
else "bye"

if (false) {

}
else {

}

if (true)


{}else

{
    1+1
}

if (1 + 2 + 4 > 9) 
    "yep"
else
    "nop"

if (true) "i"; else "you";

// else if

if (a) "hi"
else if (b) "ho"
else if (c) "he"
else "ha"

if (true) {
    12
} else if (false) {
    42
}

// If only

if (42 > 2 > 1) {

    if (8 > 4) {
        "hi"
    }
    if (9 < 4) 42
}

if ( not (true) ) "no"
)";

    IRType expected[] = {
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,
        IRType::IF,

        IRType::END_OF_FILE
    };

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
else "hi"

if {}

if true {}

if (true) "hi" else "no"
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, Whiles){
    ustring code = R"(
while (4 > 9) "hi"

while (a)

{
    "some" ++ " " ++ "output"
}

while (a or b) { "a or b\n"; } while (c)
"d"

// Do while
do "hi"; while (4 > 9) 

do

{
    "some" ++ " " ++ "output"
}

while (a)

do  { "a or b\n"; } while (a or b); do
"d"
while (c)

do do {} while (false); while (false)
)";

    IRType expected[] = {
        IRType::WHILE,
        IRType::WHILE,
        IRType::WHILE,
        IRType::WHILE,

        IRType::DO_WHILE,
        IRType::DO_WHILE,
        IRType::DO_WHILE,
        IRType::DO_WHILE,
        IRType::DO_WHILE,

        IRType::END_OF_FILE
    };

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
while true { "hi" }
while {}

// Do while
do {
"a"
}
do { 12 } while (false)
do do {} while (false) while

//do { "hi"; } while
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,

        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, ForLoops){
    ustring code = R"(
for (a : b) {

}

for (v : "hello")
    "Hi!"

for ( some_v :  some_other ) { 1+2; }

for (b : """Some
multiline
string""") 1+3

for (i : arr1)
    for (j : i)
        for (k : j) {
    k
}
)";

    IRType expected[] = {
        IRType::FOR_LOOP,
        IRType::FOR_LOOP,
        IRType::FOR_LOOP,
        IRType::FOR_LOOP,
        IRType::FOR_LOOP,

        IRType::END_OF_FILE
    };

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
for a { "hi" }
for {}
for (a) {}
for (j :) {}
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, TryCatchFinally){
    ustring code = R"(
try {

} catch (a) {

}

try ""; catch (a:Bool) "bool"; catch(a:Int) "Int"; catch(x) "some other"

try ""
catch(e: [Int]) {}
catch(e: [Int, Bool, A::Foo])
    "exit"

try ""; catch (a:Int) {
} finally "finally"

try ""; catch (a:A::C) "c"; catch(x) "some other"; finally {
    exit(0)
}

try ""
catch(e: [Int]) {}
catch(e: [Int, Bool, A::Foo])
    "e"
finally
    "finally"
)";

    IRType expected[] = {
        IRType::TRY,
        IRType::TRY,
        IRType::TRY,
        IRType::TRY,
        IRType::TRY,
        IRType::TRY,

        IRType::END_OF_FILE
    };

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
try {}
try {} catch () {}
try {} catch(b) {} finally (a) {}
try {} catch(a) ""; catch(a:) {}
try { ""; } catch (e:[Int, F.a]) {} finally {}
try { ""; } catch (e:3) {}
catch(e) {}
finally {}
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, Imports){
    ustring code = R"(
import math
import FooBar as f
import F, Boo as B, C, D as Dee

import Some::Space::Value
import So::Bo::Go as Goo, Foo::F as f1

import A,
       B::b,
       C as C1,
       D
)";

    IRType expected[] = {
        IRType::IMPORT,
        IRType::IMPORT,
        IRType::IMPORT,
        IRType::IMPORT,
        IRType::IMPORT,
        IRType::IMPORT,

        IRType::END_OF_FILE
    };

    ustring aliases[] = {
        "math",
        "f",
        "F", "B", "C", "Dee",
        "Value",
        "Goo", "f1",
        "A", "b", "C1", "D",
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    int alias_index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index]) << "Incorrect IR at index: " << index;
        if (auto imp = dyn_cast<Import>(decl)) {
            for(unsigned i = 0; i < imp->get_names().size(); i++) {
                EXPECT_TRUE(imp->get_alias(i) == aliases[alias_index]) << "Incorrect alias at index: "
                        << index << "\nExpected: " << aliases[alias_index] << "\nBut got: " << imp->get_alias(i);
                ++alias_index;
            }
        }
        index++;
    }

    delete mod;

    // Errors
ustring incorrect = R"(
import Soo::Boo.c as D
import
import 32
import "Foo"
import Foo as F as G
import Goo as Goo::Goo
import O1 as "o1"
import F::A[1]
import G::A.a::C::d
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

TEST(Parsing, Switches){
    ustring code = R"(
switch(val) {}

switch(val) {
    case 1, 0, 4: t = true
    case 8: t = false
    case 10: { 
        x += 1
        t = nil
    }
    case 11: t = nil
    default: error()
}

switch(x+1) {case 1: "aaa"; default: "cc"; case 2: "bb";}

switch(x) {
default: 1+1
}

switch  (some_value)

{


case 23: y=3; }
)";

    IRType expected[] = {
        IRType::SWITCH,
        IRType::SWITCH,
        IRType::SWITCH,
        IRType::SWITCH,
        IRType::SWITCH,

        IRType::END_OF_FILE
    };

    run_parser(code, expected, sizeof(expected)/sizeof(expected[0]));

    // Errors
ustring incorrect = R"(
switch() { case 1: "a"; }
switch {}
switch (if(a) {42}) {}
switch (a) { case "a": 2; 43; }
switch (a) { case "a": 2; default: 0; case "c": 4; default: 0}
case 1: "1"
default: true
)";

    IRType expected_incorr[] = {
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,
        IRType::RAISE,

        IRType::END_OF_FILE
    };

    run_parser_by_line(incorrect, expected_incorr, sizeof(expected_incorr)/sizeof(expected_incorr[0]));
}

}
