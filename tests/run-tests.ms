d"""
Testing script for Moss interpreter.

Example run: moss run-tests.ms -moss build/moss -test-dir tests/
"""

import subprocess

MOSS_PATH = "moss"
TEST_DIR = ""
EXTRA_FLAGS = ""

fun print_help() {
    """Moss test suite
Usage: moss run-tests.ms [-moss path_to_moss] [-test-dir path_to_test]
"""
}

fun parse_args(args) {
    i = 0
    while (i < args.length()) {
        a = args[i]
        switch (a) {
            case "-moss": {
                i += 1
                ::MOSS_PATH = args[i]
            }
            case "-test-dir": {
                i += 1
                ::TEST_DIR = args[i]
            }
            case "-extra-flags": {
                i += 1
                ::EXTRA_FLAGS = args[i]
            }
            case "-h", "--help": {
                ~print_help()
                exit(256)
            }
            default: {
                ~print("Unknown argument: " ++ a)
                exit(1)
            }
        }
        i += 1
    }
}

fun fail(name:String, msg:String) {
    ~::FAILED_TESTS.append(name)
    ~print(f"FAIL: {name}: {msg}")
}

fun run(test:String, name:String, args:String="") {
    path = ::TEST_DIR++test
    // For Windows replace slashes
    if (::TEST_DIR.length() > 0 && ::TEST_DIR[-1] == "\\")
        path = path.replace("/", "\\")
    return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} {path}")
}

fun expect_pass(test:String, name:String, expected_out:String=nil, expected_err:String=nil, args:String="") {
    result = run(test, name, args)
    // Normalize \r\n on windows to \n
    result.stdout = result.stdout.replace("\r", "")
    result.stderr = result.stderr.replace("\r", "")
    msg = "Program failed."
    failed = false
    if (result.return_code != 0) {
        msg ++= " Return code differs."
        failed = true
    }
    if (expected_out != nil) {
        if (result.stdout != expected_out) {
            msg ++= " Stdout differs."
            failed = true;
        }
    }
    if (expected_err != nil) {
        if (result.stderr != expected_err) {
            msg ++= " Stderr differs."
            failed = true;
        }
    }
    if (failed) {
        ~fail(name, msg)
        ~print(f"Command:\n--------\n{result.command}")
        ~print(f"Expected output:\n--------\n{expected_out}")
        ~print(f"Output:\n--------\n{result.stdout}")
        ~print(f"Expected error:\n--------\n{expected_err}")
        ~print(f"Error output:\n--------\n{result.stderr}")
        ~print(f"Return code: {result.return_code}")
        return false
    }
    return true
}

fun expect_fail(test:String, name:String, expected_out:String=nil, expected_err:String=nil, args:String="") {
    result = run(test, name, args)
    // Normalize \r\n on windows to \n
    result.stdout = result.stdout.replace("\r", "")
    result.stderr = result.stderr.replace("\r", "")
    msg = "Program was supposed to fail, but passed."
    failed = false
    if (result.return_code == 0) {
        msg ++= " Return code differs."
        failed = true
    }
    if (expected_out != nil) {
        if (result.stdout != expected_out) {
            msg ++= " Stdout differs."
            failed = true;
        }
    }
    if (expected_err != nil) {
        if (result.stderr != expected_err) {
            msg ++= " Stderr differs."
            failed = true;
        }
    }
    if (failed) {
        ~fail(name, msg)
        ~print(f"Command:\n--------\n{result.command}")
        ~print(f"Expected output:\n--------\n{expected_out}")
        ~print(f"Output:\n--------\n{result.stdout}")
        ~print(f"Expected error:\n--------\n{expected_err}")
        ~print(f"Error output:\n--------\n{result.stderr}")
        ~print(f"Return code: {result.return_code}")
        return false
    }
    return true
}

//---- TESTS ----//

space Tests {
// This space has to contain only functions to correctly count amount of tests

fun test_empty(name) {
    ~expect_pass("empty.ms", name, "", "")
}

fun test_output(name) {
    ~expect_pass("output.ms", name, "42, true, false\nmoss language\n13\n9\n42\n", "")
}

fun test_expressions(name) {
    ~expect_pass("expressions.ms", name, """27\n13\n261\ntrue\ntrue\nfalse\nfalse\ntrue\ntrue
true\nfalse\n9\n255\n0\n6699\n-42\nfalse\nacfc
hi\nthere\ntrue\ncaught
[4, 5, 6, 1, 2, 3]\n[1, 2, 3, 4]\n[1, 2, 3]\n[1, 2, 3, 4, 5, 6]\n[1, 2]\n[]\n[2, 4]
-----\n>+>+>+\n🐈🐈🐈
[]\n[]
[1, 2, 3, 1, 2, 3, 1, 2, 3]
[true, false]\n[true, false, true, false]
[]\n[]
[[[]], [[]], [[]]]
[[1], [2, [3]], [1], [2, [3]]]
[{\"a\": 2}, {\"a\": 2}]\n""", "")
}

fun test_variables(name) {
    ~expect_pass("variables.ms", name, """42\n42\n44\n44\n5\nMarek (me)\n25\n50\n20\n2\n8\n3
82\n:herb:🌿❗\n""", "")
}

fun test_functions(name) {
    ~expect_pass("functions.ms", name, """hi there\nnot here\nnil\n9\n11\n1false\ntest2\n42
0\n1\n123\n125\n123\n1trueanil[]1\n<function fooa with 3 overloads>
12[3, 4, 5]67\n12[3, 4, 5]67\n12[]false97\n12[3, 4, 5]false97
[]\n[1, 2, 3, 4]\n1[2]\n1[2, 3, 4]\n0[1, \"ok\", false, nil]\ntrue[1]
1\n0\n0\n42\nhello from greet\ngoo inner fun
2\nhi!\ntrue
5\n5\n5\n8\n8\n<object of class MyClass1>\n9\nFIRST\nFIRST\nFIRST
1,2,3!\n""", "")
}

fun test_function_overwriting(name) {
    ~expect_pass("function_overwriting.ms", name, "foo2\nbar3\nbar3\nf1\nf2\ng2\ng3\nga\nbaz3\nbaz3\nh1\nh2\nl2\nP2\nP3\nP4\n", "")
}

fun test_ifs(name) {
    ~expect_pass("ifs.ms", name, """0\nyes\nno\nno\nno\nno\nyes\nyes\nnil
very small\nsmall\nmedium\nbig\nvery big\nvery big\nb\n""", "")
}

fun test_whiles(name) {
    ~expect_pass("whiles.ms", name, "done\n.done\n.....done\n,,,done\n-\n1\n2\n3\n3\n", "")
}

fun test_switches(name) {
    ~expect_pass("switches.ms", name, "440-1\nhi!\ndef\n-1-1\n1111\n000\nfc1\n1\nfc1\nnone\nnone\n", "")
}

fun test_fors(name) {
    ~expect_pass("fors.ms", name, """Hello\nhi\nhi\n123\n2688\nno __next\n12345\n12345\n1234\n1234\n-10-8-6-4-2\n13579\n13579
Pos: [], Vel: []\ndone\ncaught
Some md header\nSome more\n""", "")
}

fun test_try_catch(name) {
    ~expect_pass("try_catch.ms", name, """Success\nfinally end\nCaught NameError: Unkown name!\nfinally end
Caught true!\nother\nCaught 3!\nfinally end\nin f\nfinally end\nCaught: 8
foo_int is not internal\n54a\ninner\ninner\noutter\ninner\ninner\noutter\n55\nend\n55
55\nend\n55\nend\nend\n55\nmodule end\n""", "")
}

fun test_classes(name) {
    ~expect_pass("classes.ms", name, """42\n<class Cat>\n<object of class Cat>
Vilda\nmeow\n<object of class Animal>
foo1\nfoo1
10\n56\n61\n42\n""", "")
}

fun test_attributes(name) {
    ~expect_pass("attributes.ms", name, """constructed\n56\n56\n56\n<object of class Foo>
<object of class B>\n91\n91\n91 != 65\n-1 == -1
4\n5\n4\nnil\n5\n4\nnil\ntrue\n""", "")
}

fun test_subscript_set(name) {
    ~expect_pass("subscript_set.ms", name, """Hi there
caught
[1, 2, 3]
[1, true, 3]
[1, true, true]
[false, true, true]
caught
[1, 2, 3, 4]
[-8, 2, 3, 4]
[-8, -7, -6, 4]
caught
[nil, 1, 2]
[0, 1, 2]
caught\n""", "")
}

fun test_inheritance(name) {
    ~expect_pass("inheritance.ms", name, """A\n<object of class A>\nB\n<object of class B>\nB
<object of class C>\nA\n<object of class D>\nb_call\na_call\na_call\nB\nb_call\n""", "")
}

//fun test_operator_funs(name) {
//    ~expect_pass("operator_funs.ms", name, """1111 == 1111 == 1111\nfalse == false\ntrue == true\n1000 == 1000
//990 == 990\ntrue == true\nfalse == false\n1005\n1020\n1001
//false\ntrue\ntrue\ntrue\nfalse\ntrue\n0\n1010\n1010\n1023
//0\n-1011\n-1011\n1001\n1100\n10\naH
//caught\ncaught\n""", "")
//}

fun test_lists(name) {
    ~expect_pass("lists.ms", name, """[1, 2, 3, false, \"pět\"]\n[[123], 4, [123]]
[5, 8, [0, -1, -7]]\n[5, 8, [0, -1, -7]]\n""", "")
}

fun test_dicts(name) {
    // FIXME: The dict order is not stable, needs to be fixed or the test to sort the results
    ~expect_pass("dicts.ms", name/*, """{:}
{3: \"3\", 10: \"10\"}
{nil: \"nil\", false: \"false\", 0: \"0\", true: \"true\", \"0_true\": \"0_true\"}
{1: 1, 2: 2, 2.000000: 2.000000}
caught
caught
{1: \"one\", 2: \"two\", 3: \"three\"}
{\"b\": 2, \"c\": 3, \"a\": 1}
{nil: \"n1\", false: \"f2\"}
{\"key\": 2.500000, 1: \"int\", <class Int>: [1, 2]}
{1: [1, 2, 3], 2: [4, 5, 6]}
{false: \"no\", true: \"yes\"}
{2.718000: \"e\", 3.140000: \"pi\"}
{\"key\": 42, nil: \"nothing\"}
{\"outer\": {\"inner\": \"value\"}}
{\"empty_dict\": {:}, \"empty_list\": [], \"empty_str\": \"\"}
{<object of class CustomObject>: \"custom1\", <object of class CustomObject>: \"custom2\"}
{<object of class CustomObject>: \"custom1_obj3\", <object of class CustomObject>: \"custom2\"}
caught
caught\ncaught\n3\na\nint\ncst_cls3
{\"a\": 42}\n{\"a\": 68}\n{true: true, 1: true, \"a\": 68}
true: true, 1: true, a: 68\n[\"b\", 2]\n[\"a\", 1]\n""", ""*/)
}

fun test_list_comprehension(name) {
    ~expect_pass("list_comprehension.ms", name, """[1, 2, 3]
[[1, 4], [1, 5], [1, 6], [2, 4], [2, 5], [2, 6], [3, 4], [3, 5], [3, 6]]
[1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99]
[2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97]
[\"H\", \"e\", \"l\", \"l\", \"o\", \"_\", \"t\", \"h\", \"e\", \"r\", \"e\", \"_\", \"p\", \"r\", \"o\", \"g\", \"r\", \"a\", \"m\", \"m\", \"e\", \"r\", \"!\"]
[\"H\", \"e\", \"l\", \"l\", \"o\", \"_\", \"t\", \"h\", \"e\", \"r\", \"e\", \"_\", \"p\", \"r\", \"o\", \"g\", \"r\", \"a\", \"m\", \"m\", \"e\", \"r\", \"!\"]
[[1, \"a\"], [1, \"b\"], [2, \"a\"], [2, \"b\"], [3, \"a\"], [3, \"b\"]]\n""", "")
}

fun test_enums(name) {
    ~expect_pass("enums.ms", name, """<Enum Colors>\nEnum {
  Blue,
  Red,
  Green,
  Purple
}
Red\nPurple\ncaught\nfalse\ntrue\nfalse\nfalse\ntrue\nfalse\nfalse\ncaught\ncaught
OLD\nold\n""", "")
}

fun test_spaces(name) {
    ~expect_pass("spaces.ms", name, """In FooSpace\nIn BarSpace\nI can see: FooSpace\nFooSpace\nFooSpace
BarSpace\nBarSpace\nFoo\nFoo\nAnonymous space\n<space Foo>\n99\n""", "")
}

fun test_optional_typing(name) {
    ~expect_pass("optional_typing.ms", name, """Int first\nString first\nAll other types
1\n2\n""", "")
}

fun test_indexing(name) {
    ~expect_pass("indexing.ms", name, """s\nab\noob\noob\n23true\nnil\noob
bcd
edc
fed
defabc
dcbafe\n
abv
vbavba
a\n\n
[1, 2, 3, 4, 5]
[6, 5, 4, 3, 2]
[1, 3, 5]
[6, 4, 2]
[]
[]
[1, 2, 3, 4, 5, 6]
[6, 5, 4, 3, 2, 1]
15
[5, nil, true, 3, 2, 1]
[1, 3, nil]\n""", "")
}

fun test_range_expr(name) {
    ~expect_pass("range_expr.ms", name, """[0, 1, 2]
[0, 2, 4, 6]
[]
[-2, -1, 0, 1]
[-2, 0]
[-4, -2, 0, 2]
[1, 0, -1, -2, -3]
[1]
[]
[2, 4, 6]
[0, 2, 4]
[-100, -101, -102, -103]
[0, 1, 2, 3, 4]\n""", "")
}

fun test_compound_assignment(name) {
    ~expect_pass("compound_assignment.ms", name, """I say hi\nI say hi!\nI say hi!?\n2\n4\n14\n9\n3\n6\n2
[\"a\", \"b\", \"c\"]\n[\"ab\", \"bd\", \"c\"]
[1, 2, 3]\n[1, 8, 1]
[1, 8, 1]\n[9, 8, 0]
[9, 8, 0]\n[0, 0, 0]
[1, 5, 10]\n[0.500000, 5, 2]
[0.500000, 5, 2]\n[1.000000, 5, 10]
[1.000000, 5, 10]\n[1.000000, 5, 0]\n""", "")
}

fun test_continues_and_breaks(name) {
    ~expect_pass("continues_and_breaks.ms", name, """hi\n13579\n13579
d1 d3 c1d1 d3 c2d1 d3 c3d1 d3 c4d1 d3 c5d1 d3 c6d1 d3 c7d1 d3 c8d1 d3 c9
Hi\n2468\n43210
2 + (3 [5], 5 [7], 7 [9], 9 [11], )|3 + ()|4 + ()|5 + ()|6 + ()|7 + ()|\n""", "")
}

fun test_scopes(name) {
    ~expect_pass("scopes.ms", name, """5\n6\n2\n2\nhi\ncaught\ncaught\n2\n3
-1\n12\n5\ncaught\nchanged\nalso\n4\n-5\ncaught\n3\n1
2\nhi there
-400\nhi\nhi there\n2\n""", "")
}

fun test_lambdas(name) {
    ~expect_pass("lambdas.ms", name, """foo: 42\nfoo: hi\ncaught\n8\n5-inner\noutter\n5\n> moss is great.\n5\n0
104\n-100\n""", "")
}

fun test_supers(name) {
    ~expect_pass("supers.ms", name, """B A\nA 2\nB\nD B\nC\nC\nA\nA foo 42\nVAL B\ncaught\n""", "")
}

fun test_notes(name) {
    ~expect_pass("notes.ms", name, """Text
# Title\nSome text.\nformatted!\nSome text\nmd\n# Title1
Note(md4\"hello\")\nNote(md5\"conststr\")\nNote(txt\"hello\")\nNote(md7\"text\")\n""", "")
}

fun test_converters(name) {
    ~expect_pass("converters.ms", name, """Title1
Paragraph1

Title2
Paragraph2
Hello
Foo1Class\n""", "")
}

fun test_converters_pipeline(name) {
    ~expect_pass("converters_pipeline.ms", name, """A2B:B2C:C2D:hi
caught
B2C:C2D:FromB
A22B2:B22D:txt
C22D:txt2
A32B3:B32D:txt3\n""", "", args="-f D")
}

fun test_generators(name) {
    ~expect_pass("generators.ms continue", name, """<!DOCTYPE html>
<html>
<body>
<h1> Moss generated html</h1>
<p>Hello, World!</p>
<h2> Heading 2</h2>
<p>
</p>
<b>Not correct converter</b>
<p>
</p>
<p>But works!</p>
</body>
</html>\n""", "", args="-f html")

    ~expect_pass("generators.ms exit0", name, """<!DOCTYPE html>
<html>
<body>
<h1> Moss generated html</h1>
<p>Hello, World!</p>
</body>
</html>\n""", "", args="-f html")

    // If exit with failure then we dont want to generate any output document as
    // it will be inclomplete anyway and there might be missing resources.
    ~expect_fail("generators.ms exit1", name, args="-f html")
}

fun test_strings(name) {
    ~expect_pass("strings.ms", name, """\\n\\tsdf\\0x12\\n
|
\\n\\treturn
\\b\\t\\n
*
ABCDEF
*
ABCDEF
true
true
Ω
Ω
> 🐉中ñ
""", "")

    ~expect_fail(r""" "\xA" """, "", /*rx_err="Short hexadecimal"*/ args="-e")
    ~expect_fail(r""" "\Q01" """, "", /*rx_err="Short octal"*/ args="-e")
    ~expect_fail(r""" "\uA32" """, "", /*rx_err="Short 16-bit unicode"*/ args="-e")
    ~expect_fail(r""" "\UA32845" """, "", /*rx_err="Short 32-bit unicode"*/ args="-e")

    ~expect_fail(r""" "\xAG" """, "", /*rx_err="Incorrect hexadecimal"*/ args="-e")
    ~expect_fail(r""" "\q080" """, "", /*rx_err="Incorrect octal"*/ args="-e")
    ~expect_fail(r""" "\u08Y3" """, "", /*rx_err="Incorrect 16-bit unicode"*/ args="-e")
    ~expect_fail(r""" "\U081336AZ" """, "", /*rx_err="Incorrect 32-bit unicode"*/ args="-e")
}

fun test_fstrings(name) {
    ~expect_pass("fstrings.ms", name, """v: 43 end
name = moss user

{}
hi
Jon loves coding in Moss.
3 + 4 = 7
Value: 10!
Greet: Marek!
dict = value
{Hello}
there Bob
# Title
lorem ipsum.\n""", "")
}

fun test_docstrings(name) {
    ~expect_pass("docstrings.ms", name, """Module info
Second line
Returns 42 Second comment.
Class ACls
Constructor
Some space\n""", "")
}

fun test_multivar(name) {
    ~expect_pass("multivar.ms", name, """1 2 3 4
true nil 5
caught
false 0 true 4
1 2 3 [4, 5, 6, 7]
[0, 1, 2, 3, 4, 5, 6] 7 8
q r [3, 4, 5, 6, 7, 8] t u v
1 []
[] 2
1 2 [3, 4, 5, 6, 7] 8 9
hi there []
1 2\n3 4\n5 6
1 2\n3 4\n5 6
a [2, 3, 4, 5] c d
a [2, 3, 4, 5] c d
[] 1
[] 2
caught\n""", "")
}

fun test_import_calls(name) {
    ~expect_pass("module_tests/square.ms", name, """mod1 ran!\nanon_space_value\nsquare\nmod2fun\ngot result\n25\n9\n100\n49
mod1 ran!\nanon_space_value\nsquare_all\nmod2fun\ngot result\n16\n6\nfalse\ninner_fun 1\n6\n9\ncaught\ncaught\n""", "")
}

fun test_space_imports(name) {
    ~expect_pass("space_imports.ms", name, """caught\n1-msf\ncaught\nval1\n2-msf\nval1\n3-msf\nlocal val1
val1\ninner local val1\nlocal val1\nval1\n""", "")
}

fun test_closures(name) {
    ~expect_pass("closures.ms", name, """24\nOC; Created Inner + OC; <object of class InnerClass>\n<class InnerClass>
<b><i>Hi there!</i></b>\ntriple_val\ndouble_val\n""", "")
}

fun test_implicit_calls(name) {
    ~expect_pass("implicit_calls.ms", name, """hello is my string\nbye is my string\n42\n<object of class MyNumber>
AAA - [1, 2, 3]\n""", "")
}

fun test_runtime_errors(name) {
    ~expect_pass("runtime_errors.ms", name, """OK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\n""", "")
}

fun test_range_precedence(name) {
    ~expect_pass("range_precedence.ms", name, """[[1], [3]]
[[1, 2], [3], 5, <object of class Range>]
[1, <object of class Range>]
[1, 1]
[1, \"two\", true, [8, 9], []]
6
7
[1, 3, <object of class Range>]
[<object of class Range>]
[[1, <object of class Range>]]
[[<object of class Range>]]\n""", "")
}

fun test_short_circuit_eval(name) {
    ~expect_pass("short_circuit_eval.ms", name, """true\ntrue\nhi\nthere\nfalse\nfail\nexists!\ndoes not exist.\n""", "")
}

fun test_unpacking(name) {
    ~expect_pass("unpacking.ms", name, """1 true hi
1 2 3
6 true 7
[6, true, 7] true [6, true, 7]
7 true 6
a h o j
l i d i
caught
caught
0 1 2 3 0
| a b c | 2
| 0 1 2 |
\ncaught
ABC\n123\ncalled\ncaught\ncaught\nfoov3\naBC\nAbc\n4BC
[1, 2, 3, \"hi\", [1, 2]]\n[0, 1, 2, 3, 4]\n[0, 1, 2, 3, 4]\n""", "")
}

fun test_equalities(name) {
    ~expect_pass("equalities.ms", name, """true\nfalse\ntrue\nfalse\nfalse\ncaught\nfalse\ntrue\ncaught
false\ntrue\ntrue\nfalse\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\n""", "")
}

fun test_memberships(name) {
    ~expect_pass("memberships.ms", name, """false\ntrue\nfalse\ncaught\ntrue\nfalse\ntrue\nfalse\ntrue
false\ntrue\ntrue\ntrue\nfalse\nfalse\ntrue\nfalse\ntrue\n""", "")
}

fun test_escaping(name) {
    ~expect_pass("escaping.ms", name, """hello
YES
96
hello there
Hello m'dam\\
Start\\
mid\\a\"txt\"
end\\
""", "")
}

fun test_static_methods(name) {
    ~expect_pass("static_methods.ms", name, """caught\nid, 6\ncaught\n""", "")
}

fun test_type_casting(name) {
    ~expect_pass("type_casting.ms", name, """42\ntrue\n42\n42.000000\ncaught\ncaught\ncaught\n""", "")
}

fun test_fibonacci(name) {
    ~expect_pass("fibonacci.ms", name, """1\n55\n233\n2584\n""", "")
}

fun test_factorial(name) {
    ~expect_pass("factorial.ms", name, """2432902008176640000""", "")
}

fun test_collatz(name) {
    ~expect_pass("collatz.ms", name, """21 64 32 16 8 4 2 1 
2097152 1048576 524288 262144 131072 65536 32768 16384 8192 4096 2048 1024 512 256 128 64 32 16 8 4 2 1 
22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 """, "")
}

fun test_pso(name) {
    ~expect_pass("pso.ms", name)
}

fun test_ascending_primes(name) {
    ~expect_pass("ascending_primes.ms", name, """[2, 3, 5, 7, 13, 17, 19, 23, 29, 37, 47, 59, 67, 79, 89, 127, 137, 139, 149, 157, 167, 179, 239, 257, 269, 347, 349, 359, 367, 379, 389, 457, 467, 479, 569, 1237, 1249, 1259, 1279, 1289, 1367, 1459, 1489, 1567, 1579, 1789, 2347, 2357, 2389, 2459, 2467, 2579, 2689, 2789, 3457, 3467, 3469, 4567, 4679, 4789, 5689, 12347, 12379, 12457, 12479, 12569, 12589, 12689, 13457, 13469, 13567, 13679, 13789, 15679, 23459, 23567, 23689, 23789, 25679, 34589, 34679, 123457, 123479, 124567, 124679, 125789, 134789, 145679, 234589, 235679, 235789, 245789, 345679, 345689, 1234789, 1235789, 1245689, 1456789, 12356789, 23456789]""", "")
}

fun test_lib_moss_module(name) {
    ~expect_pass("stdlib_tests/moss_module.ms", name, """<module libms>\n<space Math>\n3\ntrue\n""", "")
}

fun test_lib_constants(name) {
    // FIXME: Match regex: [0-9]\.[0-9]\.[0-9]\n
    ~expect_pass("stdlib_tests/constants.ms", name)
}

fun test_lib_vardump(name) {
    // Note: This test is sensitive to bytecodegen changes because it uses
    // hardcoded addresses
    // TODO: Use regex
    ~expect_pass("stdlib_tests/vardump.ms", name, """Int(42)
String(\"\\nhello\\n\\tthere\")
String(\"\")\nBool(true)\nNilType(nil)
List(4) [
  Int(1),
  String(\"two\"),
  Bool(true),
  List(2) [
    Int(8),
    Int(9)
  ]
]
List(0) []
List(1) [
  List(1) [
    Enum {
      A,
      B,
      C
    }
  ]
]\nEnum {}
Class MyClass : Int, Float {
  \"MyClass\": Fun(MyClass(n:[Int, Float]) @104),
  \"NAME\": String(\"myclass\"),
  \"get_n\": Fun(get_n() @114)
}\n""", "")
}

fun test_lib_print(name) {
    ~expect_pass("stdlib_tests/print.ms", name, """1 2 3 4\nhi-there.\n\ntrue\n1,2,3!\n""", "")
}

fun test_lib_type_constructors(name) {
    ~expect_pass("stdlib_tests/type_constructors.ms", name, """56\n56\n42\n10\n0\n22\n100\n0\n-8
0.000000\n2.500000\n50.000000\n15.000000
false\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue
Hi\n55\n<class String>\nnil
nil
[]\n[1, 2, 3]\n[true, nil, 2, 4]\n["H", "e", "l", "l", "o"]
[5, 6, 7, 8]\n[5, 6, 7, 8]\n[]
[0, 1, 2, 3, 4, 5]\n[0, 1, 2, 3, 4, 5]\n[]
Exception: Some exception\n""", "")
}

fun test_lib_builtin_exceptions(name) {
    ~expect_pass("stdlib_tests/builtin_exceptions.ms", name, """Caught: Exception: 
Caught: NameError: 
Caught: AttributeError: 
Caught: ModuleNotFoundError: 
Caught: TypeError: 
Caught: AssertionError: 
Caught: NotImplementedError: 
Caught: ParserError: 
Caught: SyntaxError: 
Caught: LookupError: 
Caught: IndexError: 
Caught: KeyError: 
Caught: MathError: 
Caught: DivisionByZeroError: 
Caught: ValueError: 
Caught: OSError: 
Caught: FileNotFoundError: 
Caught: EOFError: 
Caught: 0
Caught: NameError: 
Caught: SyntaxError: 
Caught: IndexError: 
Caught: KeyError: 
Caught: DivisionByZeroError: 
Caught: FileNotFoundError: \n""", "")
}

fun test_lib_ranges(name) {
    ~expect_pass("stdlib_tests/ranges.ms", name, """1\n3\n5\n100\n99\n98\n-2\n3\n8\n0\n1\n2
0\n1\n2\nend\nend\nend\n0\n1\n2\n0\n-1\n-2\n""", "")
}

fun test_lib_lists(name) {
    ~expect_pass("stdlib_tests/lists.ms", name, """2\n0\n8\ncaught
[1, 2, 3]\n[1, 2, 3, 4, [true, nil]]\n[1, 2, 3, 4, [true, nil], 9, [], true]
[1, true, 2, false]\nfalse\ntrue\n[1, 2]\n[1, 2, 3, 4, 5, 6]\n5\n3\n[1, 2, 4, 6]
[4, 3, 2, 1]\n[]\n[false, true]\n[3, 2, 1]
[1, 2, 3]\n1\n""", "")
}

fun test_lib_strings(name) {
    ~expect_pass("stdlib_tests/strings.ms", name, """3\n12\n11
dcba\ntrue
a \tb
hello\nthere madam!
HELLO! BACK TO YOU - 98&*
x1234y5--🧆
X1234Y5--🧆
Marek
---hi
1, 2, 3, 4, 5
[\"a\", 1]
6-7-8-9
h i !
*\ncaught\ntrue\ntrue\ncaught\nmoss
42\ncaught\n109
0x0\n0x2a\n0xc0ffee\n-0x2
0b1100\n0b0\n0b1\n0b1000\n-0b1000
0q10\n0q52\n-0q10
hello moss\nbonono\nxxx\nbba\nx11222111\ncdcdab\nhelloworld\nbrcdbr\nunchanged
repeat\nhello\ncafe\nHello\n✅A✅\n❌ Value
abc\na\n""", "")
}

fun test_lib_random(name) {
    ~expect_pass("stdlib_tests/random.ms", name, "", "")
}

fun test_lib_math(name) {
    ~expect_pass("stdlib_tests/math.ms", name, """1\n5\n0\n8
0.00\n0.50\n0.00\n1.00\n0.00\n-0.9
1.00\n0.50\n0.00\n-1.0\n1.00\n-0.3
0.00\n1.00\n-0.0\n0.00\n-0.0\n2.29
3\n4\n0.000000\n-100.000000\n12\n7.712600\n-0.120000
1\n123.450000\n0\n0.000000\n42\ninf\ninf\nnan\nnan
10\n5\n0\ninf\n90
16\n-128\n12\n0
3.000000\n10\n-3\n0\n-2
[2, 1]\n[59, 22]\ncaught\n2.000000\n0.800000\n[0.000000, 2.300000]\n""", "")
}

fun test_lib_args(name) {
    ~expect_pass("stdlib_tests/args.ms 1 2", name, "[\"1\", \"2\"]", "")
    ~expect_pass("stdlib_tests/args.ms", name, "[]", "")
    ~expect_pass("stdlib_tests/args.ms true someval  8  + -", name, "[\"true\", \"someval\", \"8\", \"+\", \"-\"]", "")
}

fun test_lib_types(name) {
    ~expect_pass("stdlib_tests/types.ms", name, """42\ntrue\ntrue\ncaught\ncaught
<class Int>\ntrue\ntrue\n<class Space>\n<class Type>\n<class Note>\n<class Type>
true\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\nfalse\ntrue
false\ntrue\nfalse\ntrue\nfalse\ntrue\ntrue\n""", "")
}

fun test_lib_complex(name) {
    ~expect_pass("stdlib_tests/complex.ms", name, """1-2i\n1,-2\ntrue\n5+0i\nfalse\nfalse\ntrue
-1.500000+3.200000i\n6-2i\n-0.500000+1.200000i\n-4-2i\n5-10i
true\n0.400000-2.300000i\ncaught\n-1+2i\ntrue\nfalse\nfalse\ntrue
0+0i\n0-2i\n15.000000+0i\n8+0i\n18+27i\n0+0i\n5.000000-1.500000i\ncaught
4+6i\n3-4i\n23+2i\n1.000000+2.000000i\n8+0i\n3-1i\n2-2i\n3.000000+4.000000i\n""", "")
}

fun test_lib_filters(name) {
    ~expect_pass("stdlib_tests/filters.ms", name, """true\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue
false\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse
4\nawesome\ncaught\nhi\n9\nnil\ncaught\nhi\n1\nnil\n""", "")
}

fun test_lib_attrs(name) {
    ~expect_pass("stdlib_tests/attrs.ms", name, r"""["MyValue", "__Bool", "__Float", "__Int", "__String", "foo", "x"]
["MyValue", "__Bool", "__Float", "__Int", "__String", "foo"]
["NAME", "TheirValue"]
[]
["MyValue", "SomeClas", "TheirValue", "moss", "mv", "tv"]
true, true, false, true, false, true, true, true
a, 42, scf, caught
-7, scf, scf, caught
caught, caught, caught, caught
""", "")
}

fun test_lib_enumerate(name) {
    ~expect_pass("stdlib_tests/enumerate.ms", name, """0. 1\n1. 2\n2. 3\n[0, 1][1, 2][2, 3]
[100, 9][101, 8][102, 7][103, 6][104, 5]\n""", "")
}

fun test_lib_subprocess_system_and_run(name) {
    // Note: Since the output might shuffle we need to test the outputs
    // inside of the test
    ~expect_pass("stdlib_tests/subprocess_tests/system_and_run.ms", name, "hi!\nno_cap\n", "")
}

fun test_gc_local_vars(name) {
    ~expect_pass("gc_tests/local_vars.ms", name, "done\n", """gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: ENUM(MyEnum)
gc.cpp::sweep: Deleting: ENUM_VALUE(A)
gc.cpp::sweep: Deleting: ENUM_VALUE(B)
gc.cpp::sweep: Deleting: ENUM_VALUE(C)
gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: LIST(List)\n""", args="--v5=gc.cpp::sweep --stress-test-gc")
}

fun test_gc_recursion(name) {
    if (::EXTRA_FLAGS == "--stress-test-gc")
        return
    // This test just checks that GC is triggered without any special mode
    // If this fails it might be because of changes in gc threshold (next_gc) 
    ~expect_pass("gc_tests/recursion.ms", name, "377\n", """gc.cpp::collect_garbage: Running GC
gc.cpp::collect_garbage: Finished GC\n""", args="--v1=gc.cpp::collect_garbage")
}

fun test_gc_global_dependency(name) {
    ~expect_pass("gc_tests/global_dependency.ms", name, "done\n", """gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame\n""", args="--v5=gc.cpp::sweep --stress-test-gc")
}

/*

fun test_(name) {
    ~expect_pass(".ms", name, """
""", "")
}

*/

} // Tests space
TEST_AMOUNT = attrs(Tests).length()

//---- RUNNING ----// 

fun run_test(name) {
    test_fun = getattr(Tests, "test_"++name)
    failed_amount = ::FAILED_TESTS.length()
    ::INDEX += 1
    ~print(f"[{::INDEX}/{::TEST_AMOUNT}] {name}: Running")
    ~test_fun(name)
    if (failed_amount == ::FAILED_TESTS.length())
        ~print(f"[{::INDEX}/{::TEST_AMOUNT}] {name}: OK")
    else
        ~print(f"[{::INDEX}/{::TEST_AMOUNT}] {name}: FAILED")
}

FAILED_TESTS = []
INDEX = 0

~parse_args(args)

fun run_all_tests() {
    ~run_test("empty")
    //~run_test("bc_read_write")
    //~run_test("note_options")

    ~run_test("output")
    ~run_test("expressions")
    ~run_test("variables")
    ~run_test("functions")
    ~run_test("ifs")
    ~run_test("whiles")
    ~run_test("switches")
    ~run_test("fors")
    ~run_test("try_catch")
    ~run_test("classes")
    ~run_test("attributes")
    ~run_test("subscript_set")
    ~run_test("inheritance")
    //~run_test("operator_funs")
    ~run_test("lists")
    ~run_test("list_comprehension")
    ~run_test("dicts")
    ~run_test("enums")
    ~run_test("spaces")
    ~run_test("optional_typing")
    ~run_test("indexing")
    ~run_test("range_expr")
    ~run_test("compound_assignment")
    //~run_test("calls")
    ~run_test("continues_and_breaks")
    ~run_test("scopes")
    ~run_test("lambdas")
    ~run_test("supers")
    ~run_test("notes")
    ~run_test("converters")
    ~run_test("converters_pipeline")
    ~run_test("generators")
    ~run_test("strings")
    ~run_test("fstrings")
    ~run_test("docstrings")
    ~run_test("multivar")
    ~run_test("function_overwriting")

    //~run_test("basic_import")
    ~run_test("import_calls")
    ~run_test("space_imports")
    ~run_test("closures")
    ~run_test("implicit_calls")
    //~run_test("exceptions_catch")
    ~run_test("runtime_errors")
    ~run_test("range_precedence")
    ~run_test("short_circuit_eval")
    ~run_test("unpacking")
    ~run_test("equalities")
    ~run_test("memberships")
    ~run_test("escaping")
    //~run_test("enable_code_output")
    ~run_test("static_methods")
    ~run_test("type_casting")
    //~run_test("warnings")

    ~run_test("fibonacci")
    ~run_test("factorial")
    ~run_test("collatz")
    ~run_test("pso")
    ~run_test("ascending_primes")

    // stdlib tests
    ~run_test("lib_moss_module")
    ~run_test("lib_constants")
    //~run_test("lib_exit")
    ~run_test("lib_vardump")
    ~run_test("lib_print")
    ~run_test("lib_type_constructors")
    ~run_test("lib_builtin_exceptions")
    ~run_test("lib_ranges")
    //~run_test("lib_input")
    ~run_test("lib_lists")
    ~run_test("lib_strings")
    ~run_test("lib_random")
    ~run_test("lib_math")
    //~run_test("lib_file")
    ~run_test("lib_args")
    ~run_test("lib_types")
    ~run_test("lib_complex")
    ~run_test("lib_filters")
    ~run_test("lib_attrs")
    ~run_test("lib_enumerate")

    ~run_test("lib_subprocess_system_and_run")

    // gc tests
    ~run_test("gc_local_vars")
    ~run_test("gc_recursion")
    ~run_test("gc_global_dependency")

    //repl tests
    //~run_test("repl_output")
    //~run_test("repl_exceptions")
}

if (EXTRA_FLAGS != "")
    ~print(f"Running all tests with \"{EXTRA_FLAGS}\"")
~run_all_tests()

if (FAILED_TESTS.length() != 0) {
    ~print(f"FAILED: {FAILED_TESTS.length()} test(s) did not pass")
    ~print("Failed tests:")
    for (t: FAILED_TESTS) {
        ~print("\t"++t)
    }
} else {
    // TODO: Add time measurement
    ~print(f"SUCCESS: {::INDEX} tests passed")
}