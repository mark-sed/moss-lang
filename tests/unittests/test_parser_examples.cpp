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

    new EquationNote(s) {
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

}