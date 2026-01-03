d"""
Testing script for Moss interpreter.

Example run: moss run-tests.ms -moss build/moss -test-dir tests/
"""

__VERSION = "0.1.0"

import subprocess
import sys
import time
import re

MOSS_PATH = "moss"
TEST_DIR = ""
EXTRA_FLAGS = ""
COMPILE_ALL = false
PLATFORM = sys.platform()

if (PLATFORM == sys.Platform.Linux) {
    C_OFF="\q033[0m"
    C_RED="\q033[0;31m"
    C_GREEN="\q033[0;32m"
    C_GRAY="\q033[1;30m"
} else {
    C_OFF=""
    C_RED=""
    C_GREEN=""
    C_GRAY=""
}

fun print_help() {
    f"""Moss test suite (version {::__VERSION})
Usage: moss run-tests.ms [-moss path_to_moss] [-test-dir path_to_test] [-extra-flags flags]
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
            case "--compile-all": {
                ~print(f"{C_GRAY}Compiling all tests!{C_OFF}")
                ::COMPILE_ALL = true
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
    if (not name in FAILED_TESTS)
        ~::FAILED_TESTS.append(name)
    ~print(f"{C_RED}FAIL{C_OFF}: {name}: {msg}")
}

fun rm(path:String, relative_path:Bool=true) {
    // TODO: Replace with built in rm once added
    if (relative_path)
        full_path = ::TEST_DIR++path
    else
        full_path = path
    // For Windows replace slashes
    if (::PLATFORM == sys.Platform.Windows)
        full_path = full_path.replace("/", "\\")

    if (::PLATFORM == sys.Platform.Windows)
        ~subprocess.system(f"del /f /q {full_path} 2>nul")
    else
        ~subprocess.system(f"rm -f {full_path}")
}

fun run(test:String,
        name:String,
        args:String="",
        prog_args:String="",
        use_repl:Bool=false,
        exec:Bool=false) {
    path = ::TEST_DIR++test
    // For Windows replace slashes
    if (::PLATFORM == sys.Platform.Windows) {
        path = path.replace("/", "\\")
    }
    if (exec) {
        if (::PLATFORM == sys.Platform.Windows) {
            // On Windows we cannot use ' for -e so all " have to be escaped
            test = test.replace("\"", "\\\"")
            // New lines are also an issue so replace them with ;
            test = test.replace("\n", ";")
            assert(prog_args.empty(), "Program arguments cannot be passed in EXEC mode!")
            return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} -e \"{test}\"")
        }
        return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} -e '{test}'")
    }
    if (use_repl) {
        assert(prog_args.empty(), "Program arguments cannot be passed in REPL mode!")
        return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} --use-repl-mode < {path}")
    }
    if (::COMPILE_ALL) {
        test_root_name = test[0..test.length()-3]
        cfile = test_root_name++".msb"
        ~compile(test, cfile, name, compile_only=true);
        cpath = ::TEST_DIR++cfile
        result = subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} {cpath} {prog_args}")
        ~rm(cfile)
        return result
    }
    return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} {path} {prog_args}")
}

fun compile(test:String,
            output_msb:String,
            name:String,
            args:String="",
            prog_args:String="",
            compile_only:Bool=true) {
    path = ::TEST_DIR++test
    // For Windows replace slashes
    if (::PLATFORM == sys.Platform.Windows)
        path = path.replace("/", "\\")
    com_onl = compile_only ? "--compile-only" : ""
    return subprocess.run(f"{::MOSS_PATH} -o {::TEST_DIR++output_msb} {com_onl} {args} {::EXTRA_FLAGS} {path} {prog_args}")
}

fun expect_str_eq_file(txt:String, file:String, name:String) {
    ftxt = with_open(f"{::TEST_DIR}{file}", fun(f)="\n".join(f.readlines()))
    if (txt != ftxt) {
        ~fail(name, "File comparison failed")
        ~print(f"String:\n--------\n{txt}")
        ~print(f"File:\n--------\n{ftxt}")
        return false
    }
    return true
}

fun expect_file_eq(file1:String, file2:String, name:String) {
    f1txt = with_open(f"{::TEST_DIR}{file1}", fun(f)="\n".join(f.readlines()))
    f2txt = with_open(f"{::TEST_DIR}{file2}", fun(f)="\n".join(f.readlines()))
    if (f1txt != f2txt) {
        ~fail(name, "File comparison failed")
        ~print(f"File 1:\n--------\n{f1txt}")
        ~print(f"File 2:\n--------\n{f2txt}")
        return false
    }
    return true
}

fun expect_pass(test:String,
                name:String,
                expected_out:[String,File]=nil,
                expected_err:[String,File]=nil,
                rx_out:[String,NilType]=nil,
                rx_err:[String,NilType]=nil,
                args:String="",
                prog_args:String="",
                use_repl:Bool=false,
                exec:Bool=false,
                compile_and_run:Bool=false,
                compile_only:Bool=false,
                output_msb:[String,NilType]=nil) {
    if (compile_and_run or compile_only) {
        assert(output_msb != nil, "Did not set output msb file for compile")
        result = compile(test, output_msb, name, args, prog_args, compile_only)
    } else
        result = run(test, name, args, prog_args, use_repl, exec)
    // Normalize \r\n on windows to \n
    if (::PLATFORM == sys.Platform.Windows) {
        result.stdout = result.stdout.replace("\r", "")
        result.stderr = result.stderr.replace("\r", "")
    }
    msg = "Program failed."
    failed = false
    if (result.return_code != 0) {
        msg ++= " Return code differs."
        failed = true
    }
    if (type(expected_out) == File) {
        lines = expected_out.readlines()
        expected_out = "\n".join(lines)
        // FIXME: this is an ugly fix
        expected_out++="\n"
    }
    if (type(expected_err) == File) {
        lines = expected_err.readlines()
        expected_err = "\n".join(lines)
        expected_out++="\n"
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
    if (rx_out != nil) {
        if (re.search(rx_out, result.stdout) == nil) {
            msg ++= " Stdout did not match regex."
            failed = true;
        }
    }
    if (rx_err != nil) {
        if (re.search(rx_err, result.stderr) == nil) {
            msg ++= " Stderr did not match regex."
            failed = true;
        }
    }
    if (failed) {
        ~fail(name, msg)
        ~print(f"Command:\n--------\n{result.command}")
        if (expected_out != nil)
            ~print(f"Expected output:\n--------\n{expected_out}")
        else
            ~print(f"Expected regex output:\n--------\n{rx_out}")
        ~print(f"Output:\n--------\n{result.stdout}")
        if (expected_err != nil)
            ~print(f"Expected error:\n--------\n{expected_err}")
        else
            ~print(f"Expected regex error:\n--------\n{rx_err}")
        ~print(f"Error output:\n--------\n{result.stderr}")
        ~print(f"Return code: {result.return_code}")
        return false
    }
    return true
}

fun expect_fail(test:String,
                name:String,
                expected_out:[String,File]=nil,
                expected_err:[String,File]=nil,
                rx_out:[String,NilType]=nil,
                rx_err:[String,NilType]=nil,
                args:String="",
                prog_args:String="",
                retcode_expected:Int=nil,
                exec:Bool=false,
                compile_and_run:Bool=false,
                compile_only:Bool=false,
                output_msb:[String,NilType]=nil) {
    if (compile_and_run or compile_only) {
        assert(output_msb != nil, "Did not set output msb file for compile")
        result = compile(test, output_msb, name, args, prog_args, compile_only)
    } else
        result = run(test, name, args, prog_args, false, exec)
    // Normalize \r\n on windows to \n
    if (::PLATFORM == sys.Platform.Windows) {
        result.stdout = result.stdout.replace("\r", "")
        result.stderr = result.stderr.replace("\r", "")
    }
    msg = "Expected fail did not meet expectations."
    failed = false
    if (result.return_code == 0) {
        msg ++= " Program was supposed to fail, but passed."
        failed = true
    }
    if (retcode_expected != nil) {
        if (result.return_code != retcode_expected) {
            msg ++= f" Return code ({result.return_code}) differs from expected ({retcode_expected})."
            failed = true;
        }
    }
    if (type(expected_out) == File) {
        lines = expected_out.readlines()
        expected_out = "\n".join(lines)
        expected_out++="\n"
    }
    if (type(expected_err) == File) {
        lines = expected_err.readlines()
        expected_err = "\n".join(lines)
        expected_out++="\n"
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
    if (rx_out != nil) {
        if (re.search(rx_out, result.stdout) == nil) {
            msg ++= " Stdout did not match regex."
            failed = true;
        }
    }
    if (rx_err != nil) {
        if (re.search(rx_err, result.stderr) == nil) {
            msg ++= " Stderr did not match regex."
            failed = true;
        }
    }
    if (failed) {
        ~fail(name, msg)
        ~print(f"Command:\n--------\n{result.command}")
        if (expected_out != nil)
            ~print(f"Expected output:\n--------\n{expected_out}")
        else
            ~print(f"Expected regex output:\n--------\n{rx_out}")
        ~print(f"Output:\n--------\n{result.stdout}")
        if (expected_err != nil)
            ~print(f"Expected error:\n--------\n{expected_err}")
        else
            ~print(f"Expected regex error:\n--------\n{rx_err}")
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

fun test_bc_read_write(name) {
    if (::COMPILE_ALL)
        return
    // Run and generate bc_read_write.msb
    ~expect_pass("bc_rw_output.ms", name, "Hello, World!\n", "", compile_and_run=true, output_msb="bc_read_write")
    // Run compile code
    ~expect_pass("bc_read_write.msb", name, "Hello, World!\n")

    // Cannot output bc for input bc
    ~expect_fail("bc_read_write.msb", name, "", 
        "moss: fatal error (Argument): Trying to dump bytecode for bytecode input.\n",
        compile_only=true, output_msb="test.msb", args="--use-color=0")

    // Check header
    ~expect_pass("bc_read_write.msb", name, args="--print-bc-header", rx_out="Bytecode header:\n  id:        0xff00002a\n  checksum:  0x0\n  version:   \\(0x[0-9A-Za-z]*\\) [0-9]+\\.[0-9]+\\.[0-9]+\n  timestamp: \\([0-9]+\\) .*\n")

    // Textual output
    ~expect_pass("bc_rw_output.ms", name, "Hello, World!\n", "", compile_and_run=true, output_msb="bc_read_write.txt", args="-S")
    ~expect_file_eq("bc_read_write_expected.txt", "bc_read_write.txt", name)

    ~rm("bc_read_write.msb")
    ~rm("bc_read_write.txt")
}

fun test_note_options(name) {
    // Normal run
    ~expect_pass("simple_notes.ms", name, "Start\n# Title1\nParagraph1\n\n# Title2\nParagraph2\nEnd\n", "")

    // Quiet run wihtout notes
    ~expect_pass("simple_notes.ms", name, "Start\nEnd\n", "", args="-q")
    ~expect_pass("simple_notes.ms", name, "Start\nEnd\n", "", args="--disable-notes")

    // -O option
    OUTF="._note_opts_out.ms"
    ~expect_pass("simple_notes.ms", name, "Start\nEnd\n", "", args=f"-O {::TEST_DIR}{OUTF}")
    ~expect_str_eq_file("# Title1\nParagraph1\n\n# Title2\nParagraph2", OUTF, name)
    ~rm(OUTF)

    // -O and -p
    ~expect_pass("simple_notes.ms", name, "Start\n# Title1\nParagraph1\n\n# Title2\nParagraph2\nEnd\n", "", args=f"-O {::TEST_DIR}{OUTF} -p")
    ~expect_str_eq_file("# Title1\nParagraph1\n\n# Title2\nParagraph2", OUTF, name)
    ~rm(OUTF)

    // Invalid combination
    ~expect_fail("simple_notes.ms", name, "", rx_err=".*Invalid combination of arguments.*", args=f"-q -O {::TEST_DIR}{OUTF}")
    ~expect_fail("simple_notes.ms", name, "", rx_err=".*Invalid combination of arguments.*", args=f"-q -p")
    ~rm(OUTF)
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
[{"a": 2}, {"a": 2}]\n""", "")
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

fun test_annotations(name) {
    ~expect_pass("annotations.ms", name, "caught\ncaught\ncaught\n", "")
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
55\nend\n55\nend\nend\n55\nmodule end\n3\n""", "")

    ~expect_fail(r"""try { raise 42; } catch (e) { raise e; }""", name, "", rx_err="42", exec=true)
}

fun test_finally(name) {
    ~expect_pass("finally.ms", name, """Caught\nRan finally\n1\nCaught\nRan finally\n42\nRan finally\nhi
Ran finally\n8\nin try\nin finally\nStart\nCLOSING\nhi\n\nStart\ncaught\nCLOSING\nnil
Running division\n2\nRunning division\ninf\nin try\ncaught: 42\nran finally
caught: 3\ncaught: true\ncaught: -1\nfinally 3 done\nfinally 2 done\nfinally 1 done
finally 1\nfinally 2\n2
catch\nFinally\ncaught 3\nfinally 3\ncatch2\nfinally2\n""", "")

    ~expect_fail(r"""try { } catch (e) { } finally { raise 4; }""", name, "", rx_err="4", exec=true)
    ~expect_fail(r"""try { raise 1; } catch (e) { } finally { raise 4; }""", name, "", rx_err="4", exec=true)
    ~expect_fail(r"""
        try {
            raise 4
        } catch(v:String) {
            "catch\n"
        } finally {
            "Finally\n"
        }
        """, name, "Finally\n", rx_err="4", exec=true)
}

fun test_cycles_and_exceptions(name) {
    ~expect_pass("cycles_and_exceptions.ms", name, """Exception: Got 0
Checked value: 0\n1 is odd\nChecked value: 1\nChecked value: 2\n3 is odd\nChecked value: 3\nChecked value: 4\n5 is odd\nChecked value: 5\nChecked value: 6\n7 is odd\nChecked value: 7\nChecked value: 8\n9 is odd\nChecked value: 9\nChecked value: 10\nValue is too big to check!\nChecked value: 11
2\nfinally 1\nLOOPED\nfinally 2\nDONE!
try: 0
finally after continue: 0
try: 1
finally after continue: 1
try: 2
finally after continue: 2
try: 0
finally after break: 0
catch 0
finally after catch+break: 0
inner finally: 0\nouter finally: 0\ninner finally: 1\nouter finally: 1\ninner finally: 2\nouter finally: 2
1\nloop finally inner\nloop finally outter\n2\nloop finally inner\nloop finally outter\n3\nloop finally inner\nloop finally outter\ndone looping\nfinally inner\nfinally global
1\nloop finally inner\nloop finally outter\n2\nloop finally inner\nloop finally outter\n3\nloop finally inner\nloop finally outter\ndone looping\nfinally inner\nfinally global
1\nloop finally inner\nloop finally outter\n2\nloop finally inner\nloop finally outter\n3\nloop finally inner\nloop finally outter\ndone looping\nfinally inner\nfinally global\n""", "")
}

fun test_classes(name) {
    ~expect_pass("classes.ms", name, """42\n<class Cat>\n<object of class Cat>
Vilda\nmeow\n<object of class Animal>
foo1\nfoo1
10\n56\n61\n42
42\n42 []\n42 []\n42 [1, 2, 3, 4]\n""", "")
    ~expect_fail("class MyIntString : Int, File, String {}", name, "", exec=true)
    ~expect_fail("class A {}; class B : A, Exception, ::A {}", name, "", rx_err="Duplicate base class 'A'", exec=true) // Duplicate name
}

fun test_attributes(name) {
    ~expect_pass("attributes.ms", name, """constructed\n56\n56\n56\n<object of class Foo>
<object of class B>\n91\n91\n1\n1 != 65\n-1 == -1
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
<object of class C>\nA\n<object of class D>\nb_call\na_call\na_call\nB\nb_call
Caught\nCaught\nCaught\nCaught\nCaught\nCaught\nCaught\nCaught\n""", "")
    // Cannot call without argument as MyValue is extended and does not have constructor without argument
    ~expect_fail("class MyValue { fun MyValue(x) { this.x = x; };}; class TheirValue : MyValue {}; tv = TheirValue()", name, exec=true)
    // Cannot extend NilType, nil is a true singleton value
    ~expect_fail("class MyNilType : NilType {}", name, "", exec=true)
}

fun test_operator_funs(name) {
    ~expect_pass("operator_funs.ms", name, """1111 == 1111 == 1111\nfalse == false\ntrue == true\n1000 == 1000
990 == 990\ntrue == true\nfalse == false\n1005\n1020\n1001
false\ntrue\ntrue\ntrue\nfalse\ntrue\n0\n1010\n1010\n1023
0\n-1011\n-1011\n1001\n1100\n10\naH
caught\ncaught
18\nnil\ncaught\ncaught\ncalled 3\n> called 3\n""", "")
}

fun test_lists(name) {
    ~expect_pass("lists.ms", name, """[1, 2, 3, false, \"pět\"]\n[[123], 4, [123]]
[5, 8, [0, -1, -7]]\n[5, 8, [0, -1, -7]]\n""", "")
}

fun test_dicts(name) {
    // FIXME: The dict order is not stable, needs to be fixed or the test to sort the results
    ~expect_pass("dicts.ms", name, """{:}
{10: "10", 3: "3"}
{0: "0", "0_true": "0_true", false: "false", nil: "nil", true: "true"}
{1: 1, 2: 2, 2.000000: 2.000000}
caught\ncaught
{1: "one", 2: "two", 3: "three"}
{"a": 1, "b": 2, "c": 3}
{false: "f2", nil: "n1"}
{1: "int", <class Int>: [1, 2], "key": 2.500000}
{1: [1, 2, 3], 2: [4, 5, 6]}
{false: "no", true: "yes"}
{2.718000: "e", 3.140000: "pi"}
{"key": 42, nil: "nothing"}
{"outer": {"inner": "value"}}
{"empty_dict": {:}, "empty_list": [], "empty_str": ""}
{CustomObject: 1: "custom1", CustomObject: 2: "custom2"}
{CustomObject: 1: "custom1_obj3", CustomObject: 2: "custom2"}
caught\ncaught\ncaught\n3\na\nint\ncst_cls3
{"a": 42}\n{"a": 68}\n{1: true, "a": 68, true: true}\n""", "")
}

fun test_bytes(name) {
    ~expect_pass("bytes.ms", name, """68656C6C6F\ntrue\n[b"", b"61"]\n""", "")
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

fun test_calls(name) {
    ~expect_pass("calls.ms", name, rx_out="called.*.*\n.*could not match arguments.*\n.*could not match arguments.*\n.*passed more arguments than the function accepts.*\nhi\n.*could not match arguments.*\n.*could not match arguments.*\n.*passed more arguments than the function accepts.*\n")
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
-400\nhi\nhi there\n2
011\n5\n5\n10\n""", "")
}

fun test_lambdas(name) {
    ~expect_pass("lambdas.ms", name, """foo: 42\nfoo: hi\ncaught\n8\n5-inner\noutter\n5\n> moss is great.\n5\n0
104\n-100\n""", "")
}

fun test_supers(name) {
    ~expect_pass("supers.ms", name, """B A\nA 2\nB\nD B\nC\nC\nA\nA foo 42\nVAL B\ncaught
in parent\n1\ntext\n1\ntext\ncaught\n""", "")
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

    ~expect_fail(r"""
@converter("txt", "yy")
fun toyy(x) {
    "also txt"
    return "yy"++x
}
"hi\n"
""", name, "", rx_err="Recursive converter", exec=true, args="-f yy")
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
    ~expect_pass("generators.ms", name, """<!DOCTYPE html>
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
</html>\n""", "", args="-f html", prog_args="continue")

    ~expect_pass("generators.ms", name, """<!DOCTYPE html>
<html>
<body>
<h1> Moss generated html</h1>
<p>Hello, World!</p>
</body>
</html>\n""", "", args="-f html", prog_args="exit0")

    // If exit with failure then we dont want to generate any output document as
    // it will be inclomplete anyway and there might be missing resources.
    ~expect_fail("generators.ms", name, args="-f html", prog_args="exit1")
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

    ~expect_fail(r""" "\xA" """, name, "", rx_err="Short hexadecimal", exec=true)
    ~expect_fail(r""" "\Q01" """, name, "", rx_err="Short octal", exec=true)
    ~expect_fail(r""" "\uA32" """, name, "", rx_err="Short 16-bit unicode", exec=true)
    ~expect_fail(r""" "\UA32845" """, name, "", rx_err="Short 32-bit unicode", exec=true)

    ~expect_fail(r""" "\xAG" """, name, "", rx_err="Incorrect hexadecimal", exec=true)
    ~expect_fail(r""" "\q080" """, name, "", rx_err="Incorrect octal", exec=true)
    ~expect_fail(r""" "\u08Y3" """, name, "", rx_err="Incorrect 16-bit unicode", exec=true)
    ~expect_fail(r""" "\U081336AZ" """, name, "", rx_err="Incorrect 32-bit unicode", exec=true)
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
Some space
foo(a,b)
foo(a)
foo()
Foo enum.\nHas ABC.\n""", "")
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

fun test_basic_import(name) {
    if (expect_pass("module_tests/greet_bc.ms", name, compile_only=true, output_msb="module_tests/greet_compiled.msb")) {
        ~expect_pass("module_tests/module.ms", name, """module.ms started
Hello, from greet.ms
Back in module
Hi, from msb greet
<module greet>
<module greet_compiled>
Hello, from greet.ms
greet's name: greet.ms (greet.ms)
Hello, from greet.ms
greet.ms
Ending module.ms\n""")
    }
    // Check that import spill stays in its scope
    ~expect_fail("module_tests/local_import.ms", name, "Hello, from greet.ms\ngreet.ms\n", rx_err="'NAME' is not defined")
    
    ~rm("module_tests/greet_compiled.msb")
}

fun test_import_calls(name) {
    ~expect_pass("module_tests/square.ms", name, """mod1 ran!\nanon_space_value\nsquare\nmod2fun\ngot result\n25\n9\n100\n49
mod1 ran!\nanon_space_value\nsquare_all\nmod2fun\ngot result\n16\n6\nfalse\ninner_fun 1\n6\n9\ncaught\ncaught\n""", "")
}

fun test_main_in_import(name) {
    ~expect_pass("module_tests/module_with_main.ms", name, """imported_module_with_main
module_with_main
Running main in module with main\n""", "")
}

fun test_modules_and_spaces(name) {
    ~expect_pass("module_tests/space_user.ms", name, """spaced_module FooSpace\nspace_user FooSpace
sm val1\nsm val2
foospace2
FooSpace2mod\n""", "")
}

fun test_space_imports(name) {
    ~expect_pass("space_imports.ms", name, """caught\n1-msf\ncaught\nval1\n2-msf\nval1\n3-msf\nlocal val1
val1\ninner local val1\ninner local val1\nval1\nnew val\n""", "")
}

fun test_space_extending(name) {
    ~expect_pass("space_extending.ms", name, """global\noverwritten value\noverwritten value\nnew fun
local only value\noverwritten value\noverwritten value\nnew fun\noutter fun\noverwritten value
true\ninner fun\noverwritten value\ntrue\ninner fun\noverwritten value\nnew fun\n""", "")
}

fun test_closures(name) {
    ~expect_pass("closures.ms", name, """24\nOC; Created Inner + OC; <object of class InnerClass>\n<class InnerClass>
<b><i>Hi there!</i></b>\ntriple_val\ndouble_val\n""", "")
}

fun test_implicit_calls(name) {
    ~expect_pass("implicit_calls.ms", name, """hello is my string\nbye is my string\n42\n<object of class MyNumber>
AAA - [1, 2, 3]\n""", "")
}

fun test_exceptions_catch(name) {
    ~expect_pass("exceptions_catch.ms", name, """NameError: a\nNameError: foo()\nModule not found
Assertion error\nType error\nAttribute error\nName error
Division by zero error\nFloat division by zero error\nDBZ\nFDBZ
Attribute error\nAttribute error\n""", "")
    ~expect_fail("some_name", name, "", rx_err="Name 'some_name' is not defined", exec=true, retcode_expected=1)
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
false\ntrue\ntrue\nfalse\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse
true\nfalse\nfalse\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\ntrue\n""", "")
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

fun test_enable_code_output(name) {
    exp_out = open(f"{::TEST_DIR}/enable_code_output_expected.md", "r")
    ~expect_pass(f"enable_code_output.ms", name, exp_out, "", args="-f md")
    ~exp_out.close()
}

fun test_static_methods(name) {
    ~expect_pass("static_methods.ms", name, """caught\nid, 6\ncaught\n""", "")
}

fun test_type_casting(name) {
    ~expect_pass("type_casting.ms", name, """42\ntrue\n42\n42.000000\ncaught\ncaught\ncaught\n""", "")
}

fun test_warnings(name) {
    // Warnings are from parser
    if (::COMPILE_ALL)
        return
    out_expr = """start
-9223372036854775807
9223372036854775807
inf
-0.000000
-inf\n"""
    // No -W means -W ignore
    ~expect_pass("warnings.ms", name, out_expr, "")
    ~expect_pass("warnings.ms", name, out_expr, "", args="-Wi")

    // -W error has to fail
    ~expect_fail("warnings.ms", name, args="-W error")

    // -W all outputs errors to stderr and continues
    // FIXME: This test relies on the path "tests/warnings", which is bad and does not work on Windows
    ~expect_pass("warnings.ms", name, out_expr, """\
warning: tests/warnings.ms:4:10:
      | Value '9223372036854775808' cannot fit into Int. [WDx1]
      | 
    4 | incor = -9223372036854775808 
      |          ^^^^^^^^^^^^^^^^^^^
warning: tests/warnings.ms:5:10:
      | Value '9223372036854775808' cannot fit into Int. [WDx1]
      | 
    5 | incor2 = 9223372036854775808 
      |          ^^^^^^^^^^^^^^^^^^^
warning: tests/warnings.ms:11:10:
      | Value '1e500' is too big or too close to 0 to fit into Float. [WDx2]
      | 
   11 | fincor = 1e500 
      |          ^^^^^
warning: tests/warnings.ms:14:12:
      | Value '2.5e-500' is too big or too close to 0 to fit into Float. [WDx2]
      | 
   14 | fincor2 = -2.5e-500 
      |            ^^^^^^^^
warning: tests/warnings.ms:17:2:
      | Value '1e500' is too big or too close to 0 to fit into Float. [WDx2]
      | 
   17 | -1e500 
      |  ^^^^^
""", args="-W all")
}

fun test_implicit_formats(name) {
    ~expect_pass("implicit_formats.ms", name, r"""<!DOCTYPE html>
<html>
<head>
    <link rel="stylesheet" href="">
</head>
<body>
<p>Hello!<br></p>
<p>1 &lt; 2 &amp; 3 &gt; 2<br></p>
<p># Md tags `code1` \</p>
</body>

</html>
""", "", args="-f html")
    ~expect_pass("implicit_formats.ms", name, r"""Hello\!
1 < 2 & 3 \> 2
\# Md tags \`code1\` \\""", "", args="-f md")

    ~expect_pass("example_markdown.ms", name, r"""<!DOCTYPE html>
<html>
<head>
    <link rel="stylesheet" href="">
</head>
<body>
<h1>Title</h1>
<p>Some text after title <b>with bold</b> &amp; some bad values \&quot;\&lt;&gt;\&quot;. <a href="link.com" title="">also a link</a> is a <i>link</i>.</p>
<p></p>
<ul><li><p>Table value 1</p></li><ul><li><p>Inner value 2</p></li></ul><li><p>Table value 2</p></li></ul>
<ol start="1"><li><p>Numbered list value</p></li><li><p>Numbered value list 2</p></li><ul><li><p>Inner non numbered value</p></li><li><p>An another one</p></li></ul><li><p>Last numbered value</p></li></ol>
<blockquote>Quote</blockquote>
<blockquote>And inner quote</blockquote>
<p></p>
<p><i>Here</i> is <del>a</del> <code>code</code> block:</p>
<pre><code class="language-cpp">for (l: parsed) {
    l.to_txt()
}
</code></pre>
<p></p>
<h2>Title 2 </h2>
<p></p>
<p><img src="https://example.com/cat.jpg " title="Cute cat" alt="A cat"></p>
<p></p>
<p>This is <i>italic</i> text, and this is <b>bold</b> text.</p>
<p></p>
<p>You can also <i>underline</i> (using HTML: <code>&lt;u&gt;text&lt;/u&gt;</code>).</p>
<p></p>
<p>This is <b>bold and </b><b><i>nested italic</i></b> text.</p>
<p></p>
<p>This is <code>inline code</code>.</p>
<p></p>
<p>The END.</p></body>

</html>
""", "", args="-f html") //"
}

fun test_mains(name) {
    ~expect_pass("main.ms", name, """Initialization of vars
A = 42
Initialization of vars after main def
B = 1
Running test main\n""", "")
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

fun test_lib_exit(name) {
    ~expect_pass("stdlib_tests/exit.ms", name, "caught 42\nhi\n")
    ~expect_fail("exit(\"bye\")", name, "", "bye", exec=true, retcode_expected=1)
    ~expect_fail("exit(42)", name, "", "", exec=true, retcode_expected=42)
    ~expect_fail("""
fun exit(a, b) {
    a ++ "\\n"
    ::exit(b)
}
exit("la fin", 2)""", name, "la fin\n", "", exec=true, retcode_expected=2)
    ~expect_fail("module_tests/exit_external.ms", name, "started\n", "", retcode_expected=5)
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
Class MyClass : A, B {
  "MyClass": Fun(MyClass(n:[Int, Float]) @110),
  "NAME": String("myclass"),
  "get_n": Fun(get_n(this) @119)
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
Exception: Some exception
caught\ncaught\ncaught\n""", "")
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
Caught: OutputError: 
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

fun test_lib_input(name) {
    ~expect_pass(f"stdlib_tests/input.ms", name, "> // input function test\n// this file is also read\n5//\n", "", prog_args=f"< {::TEST_DIR}/stdlib_tests/input.ms")
    ~expect_pass(f"stdlib_tests/input_eof.ms", name, "caught eof\n", "", prog_args=f"< {::TEST_DIR}/empty.ms")
}

fun test_lib_copy(name) {
    ~expect_pass("stdlib_tests/copy.ms", name, """{"a": 1}\n{"a": 2}\n{1: 2, 2: {"a": "changed"}}\n{1: nil, 2: {"a": "changed"}}
[1, 2, 3, 4, 5]\n[1, 2, 3, 4, 5, 6]\n[[5, 2, 3]]\n[[5, 2, 3]]
5, [1, 2, 3]\n-1, [1, 2, 3]\n5, [1, 2, 3]\n-1, [1, 2, 3]\n5, [1, 2, 3]\n-1, [1, 2, 3]\n5, [false, 2, 3]\n-1, [false, 2, 3]
copy of A\ncopy of A\ncaught\nchanged\nchanged\n""", "")
}

fun test_lib_lists(name) {
    ~expect_pass("stdlib_tests/lists.ms", name, """2\n0\n8\ncaught
[1, 2, 3]\n[1, 2, 3, 4, [true, nil]]\n[1, 2, 3, 4, [true, nil], 9, [], true]
[1, true, 2, false]\nfalse\ntrue\n[1, 2]\n[1, 2, 3, 4, 5, 6]\n5\n3\n[1, 2, 4, 6]
[4, 3, 2, 1]\n[]\n[false, true]\n[3, 2, 1]
[1, 2, 3]\n1\n[1, 2, 3, 4, [3, 4, 5, []]]\n[]\n[]
false\ntrue\nfalse\nfalse\ntrue
[1, 2, 3, 4, 5]\n[1, 2, 3, 4, 5, 6]\n[[5, 2, 3]]\n[[5, 2, 3]]
2\n2\n0\n1\n3\n0
1\ncaught\n0\ncaught\ncaught\n2
[0, 1, 2, 3]
[0, 1, 2, 3, 4]
[1, true, 3]
[nil]
[1, nil]
[1, nil, 2]\n""", "")
}

fun test_lib_list_delete(name) {
    ~expect_pass("stdlib_tests/list_delete.ms", name, """[0, 4, 5]
[4, 5]\n[0, 1, 2]\n[1, 3, 5, 7]\n[]\n[]
[0, 1, 2, 3, 4, 5]\ncaught
[1, 2, 3, 4, 5]\n[2, 3, 4, 5]\n[3, 4, 5]\n[4, 5]\n[5]\n[]
[0, 1, 2, 3, 4]\ncaught\n[0, 1, 2, 3]\n""", "")
}

fun test_lib_dicts(name) {
    ~expect_pass("stdlib_tests/dicts.ms", name, """{"A": 1, "B": 2, "C": 3}\n{:}\n{:}\n{:}\n{"a": 1, "b": 2, "c": 3}
2\n{"a": 1, "c": 3}\n1\n{"c": 3}\n3\n{:}\ncaught\nnil\n1\n{:}\n{"hello": [1, 2, 3]}\n{1: true, 2: false}\n{:}
caught\ntrue\nnil\nnot in\n1\ncaught\n1\ncaught\n4\nfalse\ntrue\n0\n3
{"a": 1}\n{"a": 2}\n2\nnil\n{"a": "changed"}\n{"a": "changed"}
[]\n[["lst", [1, 2, 3]]]
[]
[]
[1, true]\ncaught
caught\ncaught\n""", "")
}

fun test_lib_strings(name) {
    ~expect_pass("stdlib_tests/strings.ms", name, """3\n12\n11
dcba\ntrue
a \tb
hello\nthere madam!
HELLO! BACK TO YOU - 98&*
x1234y5--🧆
X1234y5--🧆
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
abc\na
abCDEFGHIaabC
caught
caught
caught
Hi
AAAcAAAAcAAAAAA
["hello", "there", "from", "moss"]
["hello", "there", "from\\nmoss"]
["hello", "there", "from", "moss"]
["1", "2", "3-4", "5", ""]
["1", "", "2", "", "3", "4", "", "5", "", ""]
["1", "2--3-4--5--"]
["aa", "cc", "ee", ""]
1\n3\n0\n0\n4\n0\n2
0\n-1\n7\n-1\n0\n-1\nsir\n-1\n4\n-1\n4
AABBCCD
 A - 2 Č D Α
HHHEL23-O
ČŘŽ
7ýžč
Žába!
3\n3\n3\n10\n0\n4\n4\n7\n2\n5\n6\n3\n3\n1\n2\n5\n2\n3\n2\n1\n-1\n-1\n-1\n""", "")
}

fun test_lib_string_is(name) {
    ~expect_pass("stdlib_tests/string_is.ms", name, """true\nfalse\ntrue\nfalse
----
true\nfalse\ntrue
----
false\ntrue\nfalse\nfalse\ntrue
-----
false\ntrue\nfalse\nfalse\ntrue
-----
true\nfalse\nfalse\nfalse\ntrue
-----
true\nfalse\ntrue\ntrue\nfalse
-----
false\ntrue\nfalse\nfalse\nfalse\ntrue
-----
true\nfalse\nfalse\nfalse
-----\n""", "")
}

fun test_lib_extending_string(name) {
    ~expect_pass("stdlib_tests/extending_string.ms", name, """<class MyString>\n5\nhello\ncaught\ncaught\nfalse\na
caught\ngfedcba\nAbcdefg\nABCDEFG\nabcdefg\n-bcdefg\n---defg\n1[MS],2[MS],3\nabcdefg\n123\n""", "")
}

fun test_lib_extending_note(name) {
    ~expect_pass("stdlib_tests/extending_note.ms", name, """<class MyNote>\nhello\n5\nHELLO\n""", "")
}

fun test_lib_extending_int_float(name) {
    ~expect_pass("stdlib_tests/extending_int_float.ms", name, """<class MyInt>\n42\n10
<class MyFloat>\n0.500000\n""", "")
}

fun test_lib_extending_list(name) {
    ~expect_pass("stdlib_tests/extending_list.ms", name, """<class MyList>\n[1, 2, 3, 4]\n1234\n4\nfalse
ML:[1, 2, 3, 4, 0, 1]\n3\n1\nML:[1, 2, 4, 0]\nML:[]\n""", "")
}

fun test_lib_extending_dict(name) {
    ~expect_pass("stdlib_tests/extending_dict.ms", name, """<class MyDict>\n[D]:{"b": 2, "c": 3, "a": 1}\n[D]:{:}\n1
[D]:{"b": 2, "c": 3}\n2\nfalse\nnope\n[D]:{"b": 2}\n[D]:{:}\n""", "")
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
[2, 1]\n[59, 22]\ncaught\n2.000000\n0.800000\n[0.000000, 2.300000]
[]\n[1, 3, 6, 10, 15]\n[0, 0, -10, -10, 1]\n""", "")
}

fun test_lib_file(name) {
    DATA_FILE=".data_file_read.txt"
    ~with_open(DATA_FILE, fun(f) = f.write("Czechia\nSlovakia\n\nFrance\nEngland\nGermany\nItaly"), "w")

    ~expect_pass("stdlib_tests/file.ms", name, """caught\ncaught\n<C++ value of type std::fstream>
[\"Czechia\", \"Slovakia\", \"\", \"France\", \"England\", \"Germany\", \"Italy\"]
written line
Lorem Ipsum
Lorem Ipsum
Lorem Lorem Ipsum
Lorem Ipsum
---
Lorem |Ipsum
Lorem Ipsum
end
4C6F72656D
4C6F72656D20497073756D0A4C6F72656D20497073756D0A
Lorem Ipsum
Lorem Ipsum
FFFF414243
caught
FFAA00\n""", "")

    ~rm(DATA_FILE, false)
    ~rm(".created_file_t.txt1", false)
    ~rm(".created_file_2.txt", false)
    ~rm(".created_binary_f.bytes", false)
}

fun test_lib_args(name) {
    ~expect_pass("stdlib_tests/args.ms", name, "[\"1\", \"2\"]", "", prog_args="1 2")
    ~expect_pass("stdlib_tests/args.ms", name, "[]", "")
    ~expect_pass("stdlib_tests/args.ms", name, "[\"true\", \"someval\", \"8\", \"+\", \"-\"]", "", prog_args="true someval  8  + -")
}

fun test_lib_types(name) {
    ~expect_pass("stdlib_tests/types.ms", name, """42\ntrue\ntrue\ncaught\ncaught
<class Int>\ntrue\ntrue\n<class Space>\n<class Type>\n<class Note>\n<class Type>
<class Enum>\n<Enum Foo>\n<class Type>\n<class Type>
true\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\nfalse\ntrue
false\ntrue\nfalse\ntrue\nfalse\ntrue\ntrue
true\ntrue\ntrue\n""", "")
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
4\nawesome\ncaught\nhi\n9\nnil\ncaught\nhi\n1\nnil
[1, true, [1, 2]]\n[1, 2, 2, 4]\n[5, 6, 7, 8, 9]\n[1, 2, 3, 4]\n[]\n[]\n""", "")
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

fun test_lib_globals(name) {
    ~expect_pass("stdlib_tests/globals.ms", name, "", "")
}

fun test_lib_enumerate(name) {
    ~expect_pass("stdlib_tests/enumerate.ms", name, """0. 1\n1. 2\n2. 3\n[0, 1][1, 2][2, 3]
[100, 9][101, 8][102, 7][103, 6][104, 5]\n""", "")
}

fun test_lib_iterable_funs(name) {
    ~expect_pass("stdlib_tests/iterable_funs.ms", name, """3\n0\ncaught\n5\n5\n0\n""", "")
}

fun test_lib_inspect_inspect(name) {
    ~expect_pass("stdlib_tests/inspect_tests/inspecting.ms", name, """foo(a:[Int], b:[Bool], c=nil)
foo2(a="hello", ... other)
caught
Animal(name:[String], age:[Int, Float]=0)
<anonymous>(a, b=1, c:[Bool]=false)
lambda_with_name()
En1\nCl2\nSpc\nprint\nInt\nInt
[A, B, C, D, E]\n<Enum EnB>\n[]\n""", "")
}

fun test_lib_info(name) {
    ~expect_pass("stdlib_tests/info.ms", name, """Does foo things.
Also might do baz things.
Does foo2 things.
Also might do baz2 things.
returns nil.
This function 

Does some formatting
This function can be called like this:
    `with_code(3, 5)`
There are many benefits to this, eg:
  - Is nice
  - is cool
  - awesome
When doc start on the quote line
    then the indent will stay...\n""", "")
}

fun test_lib_help(name) {
    ~expect_pass("stdlib_tests/help.ms", name, """foo(a, b)
    Does foo(a,b)

foo(a)

foo()
    Does foo()

bar(a:[Int], b=true)
    Will do bar.
    And return some val.
class MyCls
    It is my class.
    Does classy things.
enum EnB {
        A
        B
        C
        D
        E
    }
enum EmptyEnum {}
space SomeSpace
    Just some space being spaceous.
    Is tied to SomeTime.
    Might bend.
Module empty_module
    Empty module.
    Does nothing.\n""", "")
}

fun test_lib_isinstance(name) {
    ~expect_pass("stdlib_tests/isinstance.ms", name, """true\nfalse\ntrue\ntrue\nfalse\ntrue\nfalse\ncaught\nfalse\ntrue\ncaught
true\ntrue\nfalse\ntrue\ntrue\nfalse\ntrue\ncaught\ncaught\n""", "")
}

fun test_lib_zip(name) {
    ~expect_pass("stdlib_tests/zip.ms", name, """[[1, true], [2, false], [3, nil]]
[]
[[1, 1, -1], [2, 2, -2], [3, 3, -3]]
[[1], [2], [3], [4], [5]]
[[0, 0, 0], [1, 1, 1], [2, 2, 2]]
[[0, true], [1, false]]\n""", "")
}

fun test_lib_map(name) {
    ~expect_pass("stdlib_tests/map.ms", name, """["1!", "2!", "3!", "4!", "5!"]
[]
[0, 2, 4, 6]
[]
[0, 2]
[2, 4, 6]
[11, 22, 33]
[11, 22]
["ABC", "DEF", "GHI"]\n""", "")
}

fun test_lib_subprocess_system_and_run(name) {
    // Note: Since the output might shuffle we need to test the outputs
    // inside of the test
    ~expect_pass("stdlib_tests/subprocess_tests/system_and_run.ms", name, "hi!\nno_cap\n", "")
}

fun test_lib_cffi_clib(name) {
    ~expect_pass(f"stdlib_tests/cffi_tests/clib.ms", name, """<object of class CFFI>
<C++ value of type void*>
<object of class FFHandle>
Hello from C!

42
6.700000
300.000000
-3.800000
2.000000
6.000000
45.000000
Hello from moss to C!
CLIB
done\n""", "", prog_args=f"{::TEST_DIR}/stdlib_tests/cffi_tests/clib.so")
}

fun test_lib_python_module_imports(name) {
    ~expect_pass("stdlib_tests/python_tests/py_module_imports.ms", name, """caught
<object of class PythonObject>\n42\ntrue\n""", "")
}

fun test_lib_python_attrs_calls(name) {
    ~expect_pass("stdlib_tests/python_tests/py_attrs_calls.ms", name, """<object of class PythonObject>
-1.000000\n-1.000000\n1.000000\n0.000000
PythonException: module 'random' has no attribute 'NONEXISTENT_FUN'\n<object of class PythonObject>\n<object of class PythonObject>
20\n""", "")
}

fun test_lib_python_type_conversions(name) {
    ~expect_pass("stdlib_tests/python_tests/py_type_conversions.ms", name, """24\ntrue\n2.000000\nlit 😎\nnil
[1, 2, True, [], None]\n[]\n[[[42]]]\n[1, 2, [true, false], 3, 4, [nil]]
{1, 2, 3, 4, 5}\n[33, 11, 22]\n""", "")
}

fun test_lib_sys_platform(name) {
    ~expect_pass("stdlib_tests/sys_tests/platform.ms", name, "true\ntrue\n", "")
}

fun test_lib_sys_interpreter_info(name) {
    ~expect_pass("stdlib_tests/sys_tests/interpreter_info.ms", name, "", "")
}

fun test_lib_sys_environment(name) {
    ~expect_pass("stdlib_tests/sys_tests/environment.ms", name, "caught\ndoes not exist\n", "")
}

fun test_lib_time_clock_time(name) {
    ~expect_pass("stdlib_tests/time_tests/clock_time.ms", name, """time.StructTime(tm_year=2025, tm_mon=8, tm_mday=5, tm_hour=16, tm_min=55, tm_sec=36, tm_wday=1, tm_yday=217, tm_isdst=1, tm_zone="", tm_gmtoff=nil)
2025-8-5_17:26:15\ntrue\ntrue
On some august day it was: 2025-08-05 16:55:36\n""", "")
}

fun test_lib_re_match_search(name) {
    ~expect_pass("stdlib_tests/re_tests/match_search.ms", name, """<object of class Pattern>
<C++ value of type std::regex>
HELLO
<object of class Match>
["HELLO"]
["abc-123", "abc-123", "abc", "123"]
nil
["123"]
123\n""", "")
}

fun test_lib_re_flags(name) {
    ~expect_pass("stdlib_tests/re_tests/flags.ms", name, """["hello"]
["abc"]
nil
["error", "error"]
["123", "123"]
nil
nil
<object of class Match>
<object of class Match>
nil
<object of class Match>
<object of class Match>
<object of class Match>
<object of class Match>\n""", "")
}

fun test_lib_md_parser_markdown_parsing(name) {
    exp_out = open(f"{::TEST_DIR}/stdlib_tests/md_parser_tests/markdown_parsing_expected_out.txt", "r")
    ~expect_pass(f"stdlib_tests/md_parser_tests/markdown_parsing.ms", name, exp_out, "", prog_args=f"{::TEST_DIR}/stdlib_tests/md_parser_tests/all_constructs.md")
    ~exp_out.close()
}

fun test_lib_md_parser_to_txt(name) {
    ~expect_pass("stdlib_tests/md_parser_tests/to_txt.ms", name, """Title
Some text after title with bold. link.com is a link.

* Table value 1
  * Inner value 2
* Table value 2

1. Numbered list value
2. Numbered value list 2
  * Inner non numbered value
  * An another one
3. Last numbered value

Quote
And inner quote

Here is a code block:

for (l: parsed) {
    l.to_txt()
}


|A|B|C|
|1|2|3|
|0.5|1.0|3|


The END.\n""", "")
}

fun test_lib_md_parser_to_html(name) {
    ~expect_pass("stdlib_tests/md_parser_tests/to_html.ms", name, """<h1>Title</h1>
<p>Some text after title <b>with bold</b> &amp; some bad values &quot;&lt;&gt;&quot;. <a href="link.com" title="">also a link</a> is a <i>link</i>.</p>
<p></p>
<ul><li><p>Table value 1</p></li><ul><li><p>Inner value 2</p></li></ul><li><p>Table value 2</p></li></ul>
<ol start="1"><li><p>Numbered list value</p></li><li><p>Numbered value list 2</p></li><ul><li><p>Inner non numbered value</p></li><li><p>An another one</p></li></ul><li><p>Last numbered value</p></li></ol>
<blockquote>Quote</blockquote>
<blockquote>And inner quote</blockquote>
<p></p>
<p><i>Here</i> is <del>a</del> <code>code</code> block:</p>
<pre><code class="language-cpp">for (l: parsed) {
    l.to_txt()
}
</code></pre>
<p></p>
<table>
<tr>
<th style="text-align: left;">Item</th>
<th style="text-align: center;">In Stock</th>
<th style="text-align: right;">Price</th>
</tr>
<tr>
<td style="text-align: left;">Someone</td>
<td style="text-align: center;">True</td>
<td style="text-align: right;">23.99</td>
</tr>
</table>
<p></p>
<p>The END.</p>\n""", "")
}

fun test_lib_md_parser_tables(name) {
    ~expect_pass("stdlib_tests/md_parser_tests/tables.ms", name, """HEADING
PARAGRAPH
TABLE
PARAGRAPH
TABLE
PARAGRAPH
TABLE
PARAGRAPH
TABLE
PARAGRAPH
TABLE
Title

|A|B|C|D|
|1||0|3|
||3|3|0|
|a|b|0||


|T1|T2|


|Item|In Stock|Price|
|:---|:---:|---:|
|Someone|True|23.99|
|Other 24|True|23.99|
|Coders|False|19.99|
|Non coders|False|42.99|


|Month|Savings|
|---|---|
|January|$250|
|February|$80|
|March|$420|


|||
|||
\n""", "")
}

fun test_lib_csv_parser_value_parsing(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/value_parsing.ms", name, """true
[["id", "name", "age", "country_code"], ["42", "Gabriel \\"Gabi\\"", "29", "CZ"], ["22", "Jon \\"Snow\\"", "55", "US"], ["0", "Lorem\\nIpsum", "0", "NA"]]
[]
[["a", " b", " c"], ["", "", ""], [], ["hi", "there", "maam"], [], [], ["1", "2", "3"]]
[["a"], ["1"], ["2"], ["3"]]
[["a", "1", "2", "3", "4"]]\n""", "")
}

fun test_lib_csv_parser_non_numeric_parsing(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/non_numeric_parsing.ms", name, """\
[["id", "name", "age", "country_code"], [42.000000, "Marek", 29.000000, "CZ"], [22.000000, "Jon", 55.000000, "US"]]\n""", "")
}

fun test_lib_csv_parser_strict_dialect(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/strict_dialect.ms", name, """CSVError:6:7: Unterminated quote ('"').
CSVError:4:5: Varying number of values in records (from 3 to 2).
CSVError: Delimiter ('Dialects.delimiter') must be 1 character long (';;' is 2).
CSVError: Quote character ('Dialects.quote_char') must be 1 character long ('' is 0).
CSVError: Escape character ('Dialects.escape_char') must be 1 character long ('?!?!' is 4).\n""", "")
}

fun test_lib_csv_parser_dialects(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/dialects.ms", name, """[["a", "b", "c"], ["a", "3", "\\""], ["d", "3", "Peter \\"\\"Song\\"\\" "]]
[["name", "age", "year"], ["peter", "24", "2025"], ["some guy", "50", "2005"]]
[["a", "b", "c"], ["hello\\nthere", "some", "value"], ["2", "3", "4"]]\n""", "")

    // This tests \r\n parsing in excel csv's. These files have to be generated because of git's
    // new line normalization.
    COMMA_FILE = f".excel_data_test_moss_comma.generated.csv"
    COMMA_TEXT = """Test,Version,Machine,Score min,Score max\r\nio,1,Linux,1.21,4.03\r\npso,1,Linux,0.73,24.44\r\nackermann,1,Linux,1.21,8.18"""

    TAB_FILE = ".excel_data_test_moss_tab.generated.csv"
    TAB_TEXT = "Test\tVersion\tMachine\tScore min\tScore max\r\nio\t1\tLinux\t1.21\t4.03\r\npso\t1\tLinux\t0.73\t24.44\r\nackermann\t1\tLinux\t1.21\t8.18"

    ~with_open(COMMA_FILE, fun(f)=f.write(COMMA_TEXT), "w")
    ~with_open(TAB_FILE, fun(f)=f.write(TAB_TEXT), "w")

    ~expect_pass(f"stdlib_tests/csv_parser_tests/excel_dialect.ms", name,
"""[["Test", "Version", "Machine", "Score min", "Score max"], ["io", "1", "Linux", "1.21", "4.03"], ["pso", "1", "Linux", "0.73", "24.44"], ["ackermann", "1", "Linux", "1.21", "8.18"]]
[["Test", "Version", "Machine", "Score min", "Score max"], ["io", "1", "Linux", "1.21", "4.03"], ["pso", "1", "Linux", "0.73", "24.44"], ["ackermann", "1", "Linux", "1.21", "8.18"]]\n""",
    "", prog_args=f"{COMMA_FILE} {TAB_FILE}")

    ~rm(COMMA_FILE, false)
    ~rm(TAB_FILE, false)
}

fun test_lib_csv_parser_dict_reader(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/dict_reader.ms", name, """true
true
{:}\n""", "", prog_args=f"{::TEST_DIR}/stdlib_tests/csv_parser_tests/example.csv")
}

fun test_lib_csv_parser_converters(name) {
    ~expect_pass("stdlib_tests/csv_parser_tests/csv_converters.ms", name, """|A|B|C|D|
|1|2|3|4|
|5|6|7|8|
<table>
<tr>
<th>A</th>
<th>B</th>
<th>C</th>
<th>D</th>
</tr>
<tr>
<td>1</td>
<td>2</td>
<td>3</td>
<td>4</td>
</tr>
<tr>
<td>5</td>
<td>6</td>
<td>7</td>
<td>8</td>
</tr>
</table>\n""", "")

    ~expect_pass("""csv\"\"\"
A,B,C,D
1,2,3,4
5,6,7,8
\"\"\"""",  name, 
"""|A|B|C|D|
|1|2|3|4|
|5|6|7|8|\n""", exec=true, args="-f md")

    // TODO: add test for html
}

fun test_lib_csv_parser_csv_writer(name) {
    FILE1 = f"{::TEST_DIR}/.csvtest_created1.csv"
    FILE2 = f"{::TEST_DIR}/.csvtest_created2.csv"

    ~expect_pass("stdlib_tests/csv_parser_tests/csv_writer.ms", name, """\
[["Name", "Year", "Age"], ["Marty", "1990", "21"], ["Tom", "1999", "17"], ["Misha", "2012", "19"]]
true\n""", "", prog_args=f"{FILE1} {FILE2}")

    ~rm(FILE1, false)
    ~rm(FILE2, false)
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
    if ("--stress-test-gc" in ::EXTRA_FLAGS)
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

fun test_repl_output(name) {
    ~expect_pass("repl_output.ms", name, rx_out=".*moss> moss> moss> 5\nmoss> hi\n.*\n.*\n.*<object of class Cls1>\nmoss> 5\nmoss> moss> moss> moss> moss> moss>", use_repl=true)
}

fun test_repl_exceptions(name) {
    ~expect_pass("repl_exceptions.ms", name, rx_err="Stacktrace:\n  top-level.*\n.*Incorrect call.*could not match arguments.*\nStacktrace:\n  foo\\(a\\) at <repl>\n  top-level scope.*\n.*Name 'b' is not defined.*\nStacktrace:\n  top-level scope at <repl>\n.*Incorrect call to function 'foo'.*passed more arguments.*", use_repl=true)
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
        ~print(f"[{::INDEX}/{::TEST_AMOUNT}] {name}: {C_GREEN}OK{C_OFF}")
    else
        ~print(f"[{::INDEX}/{::TEST_AMOUNT}] {name}: {C_RED}FAILED{C_OFF}")
}

FAILED_TESTS = []
INDEX = 0

~parse_args(args)

fun run_all_tests() {
    ~run_test("empty")
    ~run_test("bc_read_write")
    ~run_test("note_options")

    ~run_test("output")
    ~run_test("expressions")
    ~run_test("variables")
    ~run_test("functions")
    ~run_test("ifs")
    ~run_test("whiles")
    ~run_test("switches")
    ~run_test("fors")
    ~run_test("try_catch")
    ~run_test("finally")
    ~run_test("cycles_and_exceptions")
    ~run_test("classes")
    ~run_test("attributes")
    ~run_test("subscript_set")
    ~run_test("inheritance")
    ~run_test("operator_funs")
    ~run_test("lists")
    ~run_test("list_comprehension")
    ~run_test("dicts")
    ~run_test("bytes")
    ~run_test("enums")
    ~run_test("spaces")
    ~run_test("optional_typing")
    ~run_test("indexing")
    ~run_test("range_expr")
    ~run_test("compound_assignment")
    ~run_test("calls")
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
    ~run_test("annotations")

    ~run_test("basic_import")
    ~run_test("import_calls")
    ~run_test("main_in_import")
    ~run_test("modules_and_spaces")
    ~run_test("space_imports")
    ~run_test("space_extending")
    ~run_test("closures")
    ~run_test("implicit_calls")
    ~run_test("exceptions_catch")
    ~run_test("runtime_errors")
    ~run_test("range_precedence")
    ~run_test("short_circuit_eval")
    ~run_test("unpacking")
    ~run_test("equalities")
    ~run_test("memberships")
    ~run_test("escaping")
    ~run_test("enable_code_output")
    ~run_test("static_methods")
    ~run_test("type_casting")
    ~run_test("warnings")
    ~run_test("implicit_formats")
    ~run_test("mains")

    ~run_test("fibonacci")
    ~run_test("factorial")
    ~run_test("collatz")
    ~run_test("pso")
    ~run_test("ascending_primes")

    // stdlib tests
    ~run_test("lib_moss_module")
    ~run_test("lib_constants")
    ~run_test("lib_exit")
    ~run_test("lib_vardump")
    ~run_test("lib_print")
    ~run_test("lib_type_constructors")
    ~run_test("lib_builtin_exceptions")
    ~run_test("lib_ranges")
    ~run_test("lib_input")
    ~run_test("lib_copy")
    ~run_test("lib_lists")
    ~run_test("lib_list_delete")
    ~run_test("lib_dicts")
    ~run_test("lib_strings")
    ~run_test("lib_string_is")
    ~run_test("lib_extending_string")
    ~run_test("lib_extending_note")
    ~run_test("lib_extending_int_float")
    ~run_test("lib_extending_list")
    ~run_test("lib_extending_dict")
    ~run_test("lib_random")
    ~run_test("lib_math")
    ~run_test("lib_file")
    ~run_test("lib_args")
    ~run_test("lib_types")
    ~run_test("lib_complex")
    ~run_test("lib_filters")
    ~run_test("lib_attrs")
    ~run_test("lib_globals")
    ~run_test("lib_enumerate")
    ~run_test("lib_iterable_funs")
    ~run_test("lib_info")
    ~run_test("lib_help")
    ~run_test("lib_isinstance")
    ~run_test("lib_zip")
    ~run_test("lib_map")    

    // subprocess
    ~run_test("lib_subprocess_system_and_run")

    // cffi
    if (PLATFORM != sys.Platform.Windows) {
        ~run_test("lib_cffi_clib")
    } else {
        ~print("[NOTE] Skipping cffi tests (not yet implemented on Windows).")
    }

    // python
    ~run_test("lib_python_module_imports")
    ~run_test("lib_python_attrs_calls")
    ~run_test("lib_python_type_conversions")

    // inspect
    ~run_test("lib_inspect_inspect")

    // sys
    ~run_test("lib_sys_platform")
    ~run_test("lib_sys_interpreter_info")
    ~run_test("lib_sys_environment")

    // time
    ~run_test("lib_time_clock_time")

    // re
    ~run_test("lib_re_match_search")
    ~run_test("lib_re_flags")

    // md_parser
    ~run_test("lib_md_parser_markdown_parsing")
    ~run_test("lib_md_parser_to_txt")
    ~run_test("lib_md_parser_to_html")
    ~run_test("lib_md_parser_tables")

    // csv_parser
    ~run_test("lib_csv_parser_value_parsing")
    ~run_test("lib_csv_parser_non_numeric_parsing")
    ~run_test("lib_csv_parser_strict_dialect")
    ~run_test("lib_csv_parser_dialects")
    ~run_test("lib_csv_parser_dict_reader")
    ~run_test("lib_csv_parser_converters")
    ~run_test("lib_csv_parser_csv_writer")

    // gc tests
    ~run_test("gc_local_vars")
    ~run_test("gc_recursion")
    ~run_test("gc_global_dependency")

    //repl tests
    ~run_test("repl_output")
    ~run_test("repl_exceptions")
}

if (EXTRA_FLAGS != "")
    ~print(f"{C_GRAY}Running all tests with \"{EXTRA_FLAGS}\"{C_OFF}")
// Add / or \ to ::TEST_DIR path
if (::TEST_DIR.length() != 0) {
    if(PLATFORM == sys.Platform.Windows && ::TEST_DIR[-1] != "\\") {
        ::TEST_DIR ++= "\\"
    } else if (::TEST_DIR[-1] != "/") {
        ::TEST_DIR ++= "/"
    }
} else {
    ::TEST_DIR="./"
}
start_time = time.time()
~run_all_tests()
end_time = time.time()
// TODO: Fix once float formatting is added
elapsed_time = String(round(end_time - start_time, 1))
elapsed_time = elapsed_time[0..elapsed_time.length()-5]

if (FAILED_TESTS.length() != 0) {
    ~print(f"{C_RED}FAILED{C_OFF}: {FAILED_TESTS.length()} test(s) did not pass")
    ~print("Failed tests:")
    for (t: FAILED_TESTS) {
        ~print("\t"++t)
    }
} else {
    // TODO: Add time measurement
    ~print(f"{C_GREEN}SUCCESS{C_OFF}: {::INDEX} tests passed (after {elapsed_time} s)")
}