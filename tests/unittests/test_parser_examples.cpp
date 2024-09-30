#include <gtest/gtest.h>
#include "parser.hpp"
#include "source.hpp"
#include "ir.hpp"
#include "os_interface.hpp"
#include <string>

namespace{

using namespace moss;
using namespace ir;

/** Code/snippets from syntax-and-constructs.md */
TEST(ParserExamples, SyntaxAndConstructs){
    ustring code = R"(
foo // foo = nil
foo // Will be outputted as "nil"

a = b = c = 42

fun foo() {
    return [1, 2, 3, 4, 5]
}

[e, f[0].a, g] = foo()

e // 1 
f[0].a // 2
g // [3, 4, 5]

x = 8

fun foo(x) {
    return ::x + x
}

x = 8
fun foo() {
    x = 4
    fun bar() {
        // Without $, this would create a new
        // variable x and x in foo would not
        // be overwritten
        $x = 5
    }
    bar()
    return x // 5
}

// Line comment; Ends with a new line

/*
Block comment
*/

if (condition) {
    // Code
}

if (condition) expression

if (condition) expression
else expression

if (condition) {
    // Code
} else if (condition) {
    // Code
} else {
    // Code
}

a = x ? 4 : 6

while (condition) {
    // Code
}

while (condition) expr

do {
    // Code
} while (condition)

do expr; while (condition)

for(i : 1,3..11) i // Prints:  13579

for(s: "Hello there") {
    // Code
}

switch(val) {
    case 1, 0, 4: return true
    case 8: return false
    case 10: { 
        x += 1
        return nil
    }
    case 11: return nil
    default: error()
}

import Math
import Statistics as st, Models as mod
import Math::Models as mods

import Rng::*

space SomeSpace {
    fun foo() { print("hi"); }
    space Space2 {
        fun bar() { print("ok"); }
        fun baz() { print("bye"); }
    }
}

import ::SomeSpace::*
import ::SomeSpace::Space2::baz

foo() // Foo is now accessible without scope
baz() // Also does not require scope
Space2::bar() // Requires scope for space2, since only baz was imported

fun bar() {
    raise 42
}

fun baz() {
    raise KnownException("Oops")
}

try {
    someCall()
} catch (e:KnownException) {
    e.nicemsg()
} catch (e) {
    e
} finally {
    "This is always done"
}

assert(x > 0)
assert(y < 0, "Input value should be negative.")
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    IR *mod = nullptr;

    ASSERT_EXIT( {mod = parser.parse(); exit(0); },
            testing::ExitedWithCode(0),
            "");

    delete mod;
}

}