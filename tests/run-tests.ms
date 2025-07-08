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
    return subprocess.run(f"{::MOSS_PATH} {args} {::EXTRA_FLAGS} {::TEST_DIR}{test}")
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
//
    ~run_test("output")
    ~run_test("expressions")
    ~run_test("variables")
    ~run_test("functions")
    //~run_test("ifs")
    //~run_test("whiles")
    //~run_test("switches")
    //~run_test("fors")
    //~run_test("try_catch")
    //~run_test("classes")
    //~run_test("attributes")
    //~run_test("subscript_set")
    //~run_test("inheritance")
    //~run_test("operator_funs")
    //~run_test("lists")
    //~run_test("list_comprehension")
    //~run_test("dicts")
    //~run_test("enums")
    //~run_test("space")
    //~run_test("optional_typing")
    //~run_test("indexing")
    //~run_test("range_expr")
    //~run_test("compound_assignment")
    //~run_test("calls")
    //~run_test("continues_and_breaks")
    //~run_test("scopes")
    //~run_test("lambdas")
    //~run_test("supers")
    //~run_test("notes")
    //~run_test("converters")
    //~run_test("converters_pipeline")
    //~run_test("generators")
    //~run_test("strings")
    //~run_test("fstrings")
    //~run_test("docstrings")
    //~run_test("multivar")
    ~run_test("function_overwriting")
//
    //~run_test("basic_import")
    //~run_test("import_calls")
    //~run_test("space_imports")
    //~run_test("closures")
    //~run_test("implicit_calls")
    //~run_test("exceptions_catch")
    //~run_test("runtime_errors")
    //~run_test("range_precedence")
    //~run_test("short_circuit_eval")
    //~run_test("unpacking")
    //~run_test("equalities")
    //~run_test("memberships")
    //~run_test("escaping")
    //~run_test("enable_code_output")
    //~run_test("static_methods")
    //~run_test("type_casting")
    //~run_test("warnings")
//
    //~run_test("fibonacci")
    //~run_test("factorial")
    //~run_test("collatz")
    //~run_test("pso")
    //~run_test("ascending_primes")
//
    //// stdlib tests
    //~run_test("lib_moss_module")
    //~run_test("lib_constants")
    //~run_test("lib_exit")
    //~run_test("lib_vardump")
    //~run_test("lib_print")
    //~run_test("lib_type_constructors")
    //~run_test("lib_builtin_exceptions")
    //~run_test("lib_ranges")
    //~run_test("lib_input")
    //~run_test("lib_lists")
    //~run_test("lib_strings")
    //~run_test("lib_random")
    //~run_test("lib_math")
    //~run_test("lib_file")
    //~run_test("lib_args")
    //~run_test("lib_types")
    //~run_test("lib_complex")
    //~run_test("lib_filters")
    //~run_test("lib_attrs")
    //~run_test("lib_enumerate")
//
    //~run_test("lib_subprocess_system_and_run")
//
    //// gc tests
    //~run_test("gc_local_vars")
    //~run_test("gc_recursion")
    //~run_test("gc_global_dependency")
//
    ////repl tests
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