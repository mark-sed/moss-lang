#include <gtest/gtest.h>
#include "parser.hpp"
#include "source.hpp"
#include "ir.hpp"
#include "commons.hpp"
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

/** Code/snippets from notes.md */
TEST(ParserExamples, Notes){
    ustring code = R"(
"This will be in document"

"""
This multiline
string as well.
"""

fun foo() {
    return 42
}

foo() // 42 will be in the output file

class MyClass {
    // Code
}

MyClass() // Will be converted to string and outputted

~foo() // 42 will not be in the output file

class EquationNote : Note {
    COUNT = 0

    fun EquationNote(s) {
        COUNT += 1
        this.count = COUNT
        super(s)
    }
}

@formatter
fun eq(s) {
    return EquationNote(s) 
}

eq"""
x_1 = ln_{22}(y) + 4 
"""

a = md"# Header"

@formatter
fun chess(s) {
    return s.replace("K", "♔").replace("Q", "♕").replace("R", "♖").replace("B", "♗").replace("N", "♘").replace("p", "♙")
}

chess"""
1.e4 Nf6 2.e5 Nd5 3.d4 d6 4.Nf3 g6 5.Bc4 Nb6 6.Bb3 Bg7 7.Qe2
Nc6 8.O-O O-O 9.h3 a5 10.a4 dxe5 11.dxe5 Nd4 12.Nxd4 Qxd4
13.Re1 e6 14.Nd2 Nd5 15.Nf3 Qc5 16.Qe4 Qb4 17.Bc4 Nb6 18.b3
Nxc4 19.bxc4 Re8 20.Rd1 Qc5 21.Qh4 b6 22.Be3 Qc6 23.Bh6 Bh8
24.Rd8 Bb7 25.Rad1 Bg7 26.R8d7 Rf8 27.Bxg7 Kxg7 28.R1d4 Rae8
29.Qf6+ Kg8 30.h4 h5 31.Kh2 Rc8 32.Kg3 Rce8 33.Kf4 Bc8 34.Kg5
1-0
"""

md(f"""
# {title}

__Date__: {get_localized_date()}

__Name__: {get_name()}
""")

fun header(s) = "# " ++ s

md(header("Introduction"))

import out

@converter(out::Formats::Markdown, ["tx1", "text1", "txt1"])
fun tx1_from_md(s, ... flags) {
    return s.replace("###", "<h3>").replace("##", "<h2>").replace("#", "<h1>")
}

md"""
# Introduction

This is your introduction to moss notes.
"""

import out

@converter("eq", out::Formats::LaTeX)
fun eq2tex(s) {
    return r"\begin{equation} \tag{" ++ s.count ++ "}\n" ++ s ++ r"\n\end{equation}"
}

import out

@converter("eq", out::Formats::LaTeX)
fun eq2tex(s) {
    return r"\begin{equation} \tag{" ++ s.count ++ "}\n" ++ s ++ r"\n\end{equation}"
}

@converter("eq", out::Formats::Markdown)
fun eq2md(s) {
    return f"Eq. {s.count}: `" ++ s ++ "`"
}

import out
import sys

@generator(out::Formats::LaTeX, out::Formats::PDF)
@platform(sys::Platform::Linux) // Invoked only on Linux
fun tex2pdf(s) {
    tmp = out::create_tmp(s)
    sys.system(f"pdflatex {tmp} -o {out::out_path()}")
}

import out
import sys

@generator(out::Formats::LaTeX, out::Formats::PDF)
@platform(sys::Platform::Linux) // Invoked only on Linux
fun tex2pdf(s) {
    tmp = out::create_out_tmp(s)
    // Here we use out_path() which returns path to the output file
    // that the user has chosen, but if this is not the final generator
    // in the pipeline, then this will be some dummy value
    sys.system(f"pdflatex {tmp} -o {out::out_path()}")
    return out::out_path()
}

@generator(out::Formats::PDF, "pdfa")
@platform(sys::Platform::Linux)
fun pdf2pdfa(path) {
    // Here out_path will be the actual one chosen by the user
    sys.system(f"bash pdf2pdfa.sh {path} {out::out_path()}")
}

@!notebook
// Comments are not part of the output file

md"""
This text will be in the output file.
"""

fun foo() = 42

print(foo())

fun foo() = 42

print("Hello, " ++ foo() ++ "!")

fun divxy(x:Int, y:Int) {
    d"""
    Divides x by y.
    @param x Numerator
    @param y Denominator
    @returns x divided by y
    @warning this fails if y is 0
    """
    return x / y
}

@doc_writer(out::Formats::HTML)
fun doc2html(code) {
    // Conversion code
}

import BestConverters::*
import ShinyMarkdown::md2tex

@converter("fasta", "fastq")
fun fasta2fastq(f) {
    import FastaConv
    return FastaConv::to_fastq(f)
}

md"""
# Intro

Prague is the capital of Czech Republic.
"""
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    IR *mod = nullptr;

    ASSERT_EXIT( {mod = parser.parse(); exit(0); },
            testing::ExitedWithCode(0),
            "");

    delete mod;
}

/** Code/snippets from types-values-and-operators.md */
TEST(ParserExamples, TypesAndValues){
    ustring code = R"(
"moss".upper() // "MOSS"
[1,2,3].len()  // 3
// TODO: Uncomment
//1.(+)(2)       // "3"

dec_i = 42       // Decimal
hex_i = 0x2a     // Hexadecimal
oct_i = 0q52     // Octal
bin_i = 0b101010 // Binary

// x q or b can be also capitalized
bin_j = 0B0101

x = -3.1415925
y = 0.5e-2      // 0.005
z = 5e3         // 500.0
j = 0.e+3       // 0.0

not_a_number = Float("nan")
infinity_value = Float("inf")

a = Complex(1.2, 3) // 1.2+3i
b = Complex(1)      // 1+0i
c = Complex("2-1i") // 2-1i

b1 = true
b2 = false

b3 = b1 or b2
b4 = b1 == true

nl = r"New line is written as \n"

name = "Jon"
lang = "Moss"
msg = f"{name} loves coding in {lang}"
msg // "Jon loves coding in Moss"

help = """Program usage:
\tmoss [interpret options] <input file> [program args]
"""
help // "Program usage:\n\tmoss [interpret options] <input file> [program args]\n"

a       // nil
b = nil // nil
a == b  // true

enum COLORS {
    PURPLE,
    YELLOW,
    GREEN
}

flower = COLORS::PURPLE
moss = COLORS::GREEN

names = ["luke", "leia", "han"]
mix = ["pirate", 42, true, nil, nil, "ok"]

l1 = [1, 2, 3]
l2 = [4, 5, 6]
ziped = [[x, y] : x = l1, y = l2] 
// [[1, 4],[2, 5],[3, 6]]

[a : a = (1,3..100)] // odd numbers from 1 to 99

[p if(all([p % x != 0 : x = (2..p/2)])) : p = (2..1000)] // primes

greet = "Hello there programmer!"
[c if(c != " ") else "_" : c = greet].join() // "Hello_there_programmer!"
// Alternatively one can use ternary operator for this case as well
[(c != " ") ? c : "_" : c = greet].join() // "Hello_there_programmer!"

empty = {:}
mappings = {"name": "Marek", "id": 42}

foo(1,2..5) // this is equivalent to foo(1, (2..5))
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    IR *mod = nullptr;

    ASSERT_EXIT( {mod = parser.parse(); exit(0); },
            testing::ExitedWithCode(0),
            "");

    delete mod;
}

/** 
 * Code/snippets from:
 *   functions.md and
 *   classes-and-spaces.md and
 *   annotations.md 
 */
TEST(ParserExamples, Functions_ClassesAndSpaces_Annotations){
    ustring code = R"(
fun foo(arg1, arg2) {
    return arg1 * arg2
}

fun foo(arg1) = arg1 * 2

fun(arg1) = arg1 * 2

fun getID(x:MyClass) {
    return -1
}

fun getID(x:[Int, Bool]) {
    return 0
}

fun getID(x) {
    return 42
}

fun bar(a:Int=4) {
    // Code
}

fun baz(a, b:Int, ... other, name="") {
    // Code
}

class MyClass {

    /* TODO: Uncomment
    fun (+)(x:[Int,Float]) = this.x + x
    fun (+)(x) {
        error("Cannot add to MyClass type " ++ type(x))
    }

    fun (and)(x) = this.x > 0*/

}

// Classes and Spaces

space Math {
    fun abs(x:Int) {
        return x 
    }
}

space {
    a = someCall()
    a // A is printed and unaccessible
}

class Range : Iterable, BaseClass {
    fun Range(start, end, step=1) {
        this.start = start
        this.end = end
        this.step = step
        this.i = start
    }

    fun __next() {
        if(this.step >= 0 and this.i >= this.end) return StopIteration
        if(this.step < 0 and this.i <= this.end) return StopIteration
        r = this.i
        this.i += this.step
        return r
    }
}

Range::__next(range)

@!min_version("1.1") // Module annotation

@equation // Tied to `f`
@hidden   // Tied fo `f`
fun f(x) {
    return x ^ 2 * x
}

fun g(x) {
    @!equation // Tied to `g`
    @!hidden   // Tied fo `g`
    return x * x
}
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    IR *mod = nullptr;

    ASSERT_EXIT( {mod = parser.parse(); exit(0); },
            testing::ExitedWithCode(0),
            "");

    delete mod;
}

/** Code/snippets from bytecode-and-program-sharing.md and interoperability-with-python.md*/
TEST(ParserExamples, BytecodeAndProgramSharing_PyInterop){
    ustring code = R"(
import systemx as sx

a = sx.a + 430
b = 8

fun foo(a: [Int, Float]) {
    print(a)
}

fun foo(a: SpecialString, b) {
    print(a)
    return b
}

~foo(11)
a = foo("hi")

class MyClass : XClass {

    NAME = "My Class"

    fun MyClass(a) {
        this.a = a
    }

    fun get_a() = this.a
}

myc = MyClass(42)
print(myc.get_a())

md"""
# Project Moss
Very cool stuff!
"""

@requires(1)
fun foo() {
    d"""
    Does nothing
    """
}

a = 6
if(a < 2) {
    a = 2
}
else a = 0

switch(a) {
case 1, 2: return 4
case 3: return 0
default: return -1
}

enum Colors {
    BLACK
    WHITE
}

import python

datetime = python.module("datetime")
now = datetime::datetime.now()

print(now)
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