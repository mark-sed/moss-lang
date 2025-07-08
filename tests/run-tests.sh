#!/usr/bin/env bash

# Script for running moss tests and checking the results
# example run: bash tests/run-tests.sh -moss build/moss -test-dir tests/

MOSS="moss"
OUTP_ERR=/tmp/.moss_test_err.txt
OUTP_STD=/tmp/.moss_test_std.txt
TEST_DIR="" # Directory where the test are located
POSITIONAL_ARGS=()
RUN_TEST_FLAGS="${RUN_TEST_FLAGS:=}" # Additional flags for run

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            printf "Moss test suite.\nUsage: bash $0\n" 
            exit 256
            ;;
        -moss)
            MOSS="$2"
            shift
            shift
            ;;
        -test-dir)
            TEST_DIR="$2"
            shift
            shift
            ;;
        *)
            echo "ERROR: Unknown argument: '$1'"
            exit 1
            #POSITIONAL_ARGS+=("$1")
            #shift
            ;;
    esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

FAILED_TESTS=""
INDEX=0
C_OFF='\033[0m'
C_RED='\033[0;31m'
C_GREEN='\033[0;32m'
C_GRAY='\033[1;30m'

function failed {
    FAILED_TESTS+=" $1"
    printf "${C_RED}FAIL${C_OFF}: $1: $2\n"
}

function run {
    CMD="$MOSS ${RUN_TEST_FLAGS} ${TEST_DIR}$1"
    $CMD 2>$OUTP_ERR 1>$OUTP_STD
    RETCODE=$(echo $?)
}

function run_exec {
    CMD="$MOSS -e $1"
    $MOSS -e "$1" 2>$OUTP_ERR 1>$OUTP_STD
    RETCODE=$(echo $?)
}

function run_log {
    CMD="$MOSS ${RUN_TEST_FLAGS} ${@:2} ${TEST_DIR}$1"
    $CMD &>$OUTP_STD
    RETCODE=$(echo $?)
}

function run_repl {
    CMD="$MOSS --use-repl-mode < ${TEST_DIR}$1"
    $MOSS --use-repl-mode < ${TEST_DIR}$1 2>$OUTP_ERR 1>$OUTP_STD
    RETCODE=$(echo $?)
}

function run_redir {
    CMD="$MOSS ${RUN_TEST_FLAGS} ${TEST_DIR}$1"
    $CMD < ${TEST_DIR}$2 2>$OUTP_ERR 1>$OUTP_STD
    RETCODE=$(echo $?)
}

function run_compile {
    CMD="$MOSS -o ${TEST_DIR}$2 ${TEST_DIR}$1"
    $MOSS -o ${TEST_DIR}$2 ${TEST_DIR}$1 2>$OUTP_ERR 1>$OUTP_STD
    RETCODE=$(echo $?)
}

function expect_pass {
    run "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
    fi
}

function expect_pass_exec {
    run_exec "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
    fi
}

function expect_pass_repl {
    run_repl "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
    fi
}

function expect_pass_redir {
    run_redir "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
    fi
}

function expect_pass_compile {
    run_compile "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program compilation failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
    fi
}

function expect_pass_log {
    run_log "${@:1:$#-1}"
    if [[ $RETCODE -ne 0 ]]; then
        failed "${@: -1}" "Program failed."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
    fi
}


function expect_fail {
    run $1
    local errmsg=$(cat "$OUTP_ERR")
    if [[ $RETCODE -eq 0 ]]; then
        failed $3 "Test was supposed to fail, but passed."
    elif ! [[ "$errmsg" =~ "$2" ]] ; then
        failed $3 "Error message differs."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
        printf "Expected error regex:\n--------------\n${2}\n"
    fi
}

function expect_retcode {
    if [[ $RETCODE -ne $1 ]]; then
        failed "${2}" "Return code differs."
        printf "Expected return code: $1\n"
        printf "Got return code: $RETCODE\n"
    fi
}

function expect_fail_exec {
    run_exec "$1"
    local errmsg=$(cat "$OUTP_ERR")
    if [[ $RETCODE -eq 0 ]]; then
        failed $3 "Test was supposed to fail, but passed."
    elif ! [[ "$errmsg" =~ "$2" ]] ; then
        failed $3 "Error message differs."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Error output:\n-------------\n"
        cat $OUTP_ERR
        printf "Expected error regex:\n--------------\n${2}\n"
    fi
}

function expect_fail_log {
    run_log "${@:1:$#-2}"
    local errmsg=$(cat "$OUTP_STD")
    if [[ $RETCODE -eq 0 ]]; then
        failed "${@: -1}" "Test was supposed to fail, but passed."
    elif ! [[ "$errmsg" =~ "${*: -2:1}" ]] ; then
        failed "${@: -1}" "Error message differs."
        printf "Command:\n--------\n$CMD\n"
        printf "Output:\n-------\n"
        cat $OUTP_STD
        printf "Expected error regex:\n--------------\n"
        printf "${*: -2:1}"
        printf "\n"
    fi
}

function expect_out_eq {
    outstr=$(cat $OUTP_STD)
    if ! cmp  "$OUTP_STD" <(printf "$1") ; then
        failed $2 "Output differs"
        printf "Expected:\n-------\n${1}\n"
        printf "Got:\n----\n${outstr}\n"
    fi
}

function expect_out_eq_rx {
    outstr=$(cat $OUTP_STD)
    cat $OUTP_STD | grep -zoP "$1" &>/dev/null
    if [[ $? -ne 0 ]]; then
        failed $2 "Output differs"
        printf "Expected:\n-------\n${1}\n"
        printf "Got:\n----\n${outstr}\n"
    fi
}

function expect_err_eq_rx {
    outstr=$(cat $OUTP_ERR)
    cat $OUTP_ERR | grep -zoP "$1" &>/dev/null
    if [[ $? -ne 0 ]]; then
        failed $2 "Error output differs"
        printf "Expected:\n-------\n${1}\n"
        printf "Got:\n----\n${outstr}\n"
    fi
}


function expect_err_eq {
    outstr=$(cat $OUTP_ERR)
    if ! cmp  "$OUTP_ERR" <(printf "$1") ; then
        failed $2 "Output differs"
        printf "Expected:\n-------\n${1}\n"
        printf "Got:\n----\n${outstr}\n"
    fi
}
function expect_file_eq {
    res=$(diff $1 $2)
    if [[ $? -ne 0 ]]; then
        failed $3 "Output differs"
        printf "Expected:\n-------\n"
        cat $1
        printf "Got:\n----\n"
        cat $2
        printf "Diff:\n-----\n"
        echo "$res"
    fi
}

function expect_file_eq_str {
    file_content=$(< "$2")
    expected=$(echo -e "$1")
    if [[ "$file_content" != "$expected" ]]; then
        failed $3 "Output differs"
        printf "Expected:\n-------\n"
        echo "$expected"
        printf "Got:\n----\n"
        echo "$file_content"
    fi
}

function testnum {
    printf "[${INDEX}/${TEST_AMOUNT}]"
}

function run_test {
    let "INDEX=INDEX+1"
    failed_am=${#FAILED_TESTS}
    printf "$(testnum) ${1}: Running\n"
    test_"$1" "$1"
    failed_now=${#FAILED_TESTS}
    if [[ $failed_now -eq $failed_am ]]; then
        printf "$(testnum) ${1}: ${C_GREEN}OK${C_OFF}\n"
    else
        printf "$(testnum) ${1}: ${C_RED}FAILED${C_OFF}\n"
    fi
}

###--- TESTS ---###

function test_empty {
    expect_pass "empty.ms" $1
    expect_out_eq "" $1
}

function test_output {
    expect_pass "output.ms" $1
    expect_out_eq "42, true, false\nmoss language\n13\n9\n42\n" $1
}

function test_expressions {
    expect_pass "expressions.ms" $1
    expect_out_eq "27\n13\n261\ntrue\ntrue\nfalse\nfalse\ntrue\ntrue
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
[{\"a\": 2}, {\"a\": 2}]\n" $1
}

function test_variables {
    expect_pass "variables.ms" $1
    expect_out_eq "42\n42\n44\n44\n5\nMarek (me)\n25\n50\n20\n2\n8\n3
82\n:herb:🌿❗\n" $1
}

function test_functions {
    expect_pass "functions.ms" $1
    expect_out_eq "hi there\nnot here\nnil\n9\n11\n1false\ntest2\n42
0\n1\n123\n125\n123\n1trueanil[]1\n<function fooa with 3 overloads>
12[3, 4, 5]67\n12[3, 4, 5]67\n12[]false97\n12[3, 4, 5]false97
[]\n[1, 2, 3, 4]\n1[2]\n1[2, 3, 4]\n0[1, \"ok\", false, nil]\ntrue[1]
1\n0\n0\n42\nhello from greet\ngoo inner fun
2\nhi!\ntrue
5\n5\n5\n8\n8\n<object of class MyClass1>\n9\nFIRST\nFIRST\nFIRST
1,2,3!\n" $1
}

function test_function_overwriting {
    expect_pass "function_overwriting.ms" $1
    expect_out_eq "foo2\nbar3\nbar3\nf1\nf2\ng2\ng3\nga\nbaz3\nbaz3\nh1\nh2\nl2\nP2\nP3\nP4\n" $1
}

function test_ifs {
    expect_pass "ifs.ms" $1
    expect_out_eq "0\nyes\nno\nno\nno\nno\nyes\nyes\nnil
very small\nsmall\nmedium\nbig\nvery big\nvery big\nb\n" $1
}

function test_whiles {
    expect_pass "whiles.ms" $1
    expect_out_eq "done\n.done\n.....done\n,,,done
-\n1\n2\n3\n3\n" $1
}

function test_switches {
    expect_pass "switches.ms" $1
    expect_out_eq "440-1\nhi!\ndef\n-1-1\n1111\n000
fc1\n1\nfc1\nnone\nnone\n" $1
}

function test_fors {
    expect_pass "fors.ms" $1
    expect_out_eq "Hello\nhi\nhi\n123\n2688\nno __next\n12345\n12345\n1234\n1234\n-10-8-6-4-2\n13579\n13579
Pos: [], Vel: []\ndone\ncaught
Some md header\nSome more\n" $1
}

function test_try_catch {
    expect_pass "try_catch.ms" $1
    expect_out_eq "Success\nfinally end\nCaught NameError: Unkown name!\nfinally end
Caught true!\nother\nCaught 3!\nfinally end\nin f\nfinally end\nCaught: 8
foo_int is not internal\n54a\ninner\ninner\noutter\ninner\ninner\noutter\n55\nend\n55
55\nend\n55\nend\nend\n55\nmodule end\n" $1
}

function test_classes {
    expect_pass "classes.ms" $1
    expect_out_eq "42\n<class Cat>\n<object of class Cat>
Vilda\nmeow\n<object of class Animal>
foo1\nfoo1
10\n56\n61\n42\n" $1
}

function test_attributes {
    expect_pass "attributes.ms" $1
    expect_out_eq "constructed\n56\n56\n56\n<object of class Foo>
<object of class B>\n91\n91\n91 != 65\n-1 == -1
4\n5\n4\nnil\n5\n4\nnil\ntrue\n" $1
}

function test_subscript_set {
    expect_pass "subscript_set.ms" $1
    expect_out_eq "Hi there
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
caught\n" $1
}

function test_inheritance {
    expect_pass "inheritance.ms" $1
    expect_out_eq "A\n<object of class A>\nB\n<object of class B>\nB
<object of class C>\nA\n<object of class D>\nb_call\na_call\na_call\nB\nb_call\n" $1
}

function test_operator_funs {
    expect_pass "operator_funs.ms" $1
    expect_out_eq "1111 == 1111 == 1111\nfalse == false\ntrue == true\n1000 == 1000
990 == 990\ntrue == true\nfalse == false\n1005\n1020\n1001
false\ntrue\ntrue\ntrue\nfalse\ntrue\n0\n1010\n1010\n1023
0\n-1011\n-1011\n1001\n1100\n10\naH
caught\ncaught\n" $1
}

function test_lists {
    expect_pass "lists.ms" $1
    expect_out_eq "[1, 2, 3, false, \"pět\"]\n[[123], 4, [123]]
[5, 8, [0, -1, -7]]\n[5, 8, [0, -1, -7]]\n" $1
}

function test_dicts {
    expect_pass "dicts.ms" $1
    expect_out_eq "{:}
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
true: true, 1: true, a: 68\n[\"b\", 2]\n[\"a\", 1]\n" $1
}

function test_list_comprehension {
    expect_pass "list_comprehension.ms" $1
    expect_out_eq "[1, 2, 3]
[[1, 4], [1, 5], [1, 6], [2, 4], [2, 5], [2, 6], [3, 4], [3, 5], [3, 6]]
[1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99]
[2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97]
[\"H\", \"e\", \"l\", \"l\", \"o\", \"_\", \"t\", \"h\", \"e\", \"r\", \"e\", \"_\", \"p\", \"r\", \"o\", \"g\", \"r\", \"a\", \"m\", \"m\", \"e\", \"r\", \"!\"]
[\"H\", \"e\", \"l\", \"l\", \"o\", \"_\", \"t\", \"h\", \"e\", \"r\", \"e\", \"_\", \"p\", \"r\", \"o\", \"g\", \"r\", \"a\", \"m\", \"m\", \"e\", \"r\", \"!\"]
[[1, \"a\"], [1, \"b\"], [2, \"a\"], [2, \"b\"], [3, \"a\"], [3, \"b\"]]\n" $1
}

function test_enums {
    expect_pass "enums.ms" $1
    expect_out_eq "<Enum Colors>\nEnum {
  Blue,
  Red,
  Green,
  Purple
}
Red\nPurple\ncaught\nfalse\ntrue\nfalse\nfalse\ntrue\nfalse\nfalse\ncaught\ncaught
OLD\nold\n" $1
}

function test_space {
    expect_pass "spaces.ms" $1
    expect_out_eq "In FooSpace\nIn BarSpace\nI can see: FooSpace\nFooSpace\nFooSpace
BarSpace\nBarSpace\nFoo\nFoo\nAnonymous space\n<space Foo>\n99\n" $1
}

function test_optional_typing {
    expect_pass "optional_typing.ms" $1
    expect_out_eq "Int first\nString first\nAll other types
1\n2\n" $1
}

function test_indexing {
    expect_pass "indexing.ms" $1
    expect_out_eq "s\nab\noob\noob\n23true\nnil\noob
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
[1, 3, nil]\n" $1
}

function test_range_expr {
    expect_pass "range_expr.ms" $1
    expect_out_eq "[0, 1, 2]
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
[0, 1, 2, 3, 4]\n" $1
}

function test_compound_assignment {
    expect_pass "compound_assignment.ms" $1
    expect_out_eq "I say hi\nI say hi!\nI say hi!?\n2\n4\n14\n9\n3\n6\n2
[\"a\", \"b\", \"c\"]\n[\"ab\", \"bd\", \"c\"]
[1, 2, 3]\n[1, 8, 1]
[1, 8, 1]\n[9, 8, 0]
[9, 8, 0]\n[0, 0, 0]
[1, 5, 10]\n[0.500000, 5, 2]
[0.500000, 5, 2]\n[1.000000, 5, 10]
[1.000000, 5, 10]\n[1.000000, 5, 0]\n" $1
}

function test_calls {
    expect_pass "calls.ms" $1
    expect_out_eq_rx "called.*.*\n.*could not match arguments.*\n.*could not match arguments.*\n.*passed more arguments than the function accepts.*\nhi\n.*could not match arguments.*\n.*could not match arguments.*\n.*passed more arguments than the function accepts.*\n" $1
}

function test_continues_and_breaks {
    expect_pass "continues_and_breaks.ms" $1
    expect_out_eq "hi\n13579\n13579
d1 d3 c1d1 d3 c2d1 d3 c3d1 d3 c4d1 d3 c5d1 d3 c6d1 d3 c7d1 d3 c8d1 d3 c9
Hi\n2468\n43210
2 + (3 [5], 5 [7], 7 [9], 9 [11], )|3 + ()|4 + ()|5 + ()|6 + ()|7 + ()|\n" $1
}

function test_scopes {
    expect_pass "scopes.ms" $1
    expect_out_eq "5\n6\n2\n2\nhi\ncaught\ncaught\n2\n3
-1\n12\n5\ncaught\nchanged\nalso\n4\n-5\ncaught\n3\n1
2\nhi there
-400\nhi\nhi there\n2\n" $1
}

function test_lambdas {
    expect_pass "lambdas.ms" $1
    expect_out_eq "foo: 42\nfoo: hi\ncaught\n8\n5-inner\noutter\n5\n> moss is great.\n5\n0
104\n-100\n" $1
}

function test_supers {
    expect_pass "supers.ms" $1
    expect_out_eq "B A\nA 2\nB\nD B\nC\nC\nA\nA foo 42\nVAL B\ncaught\n" $1
}

function test_notes {
    expect_pass "notes.ms" $1
    expect_out_eq "Text
# Title\nSome text.\nformatted!\nSome text\nmd\n# Title1
Note(md4\"hello\")\nNote(md5\"conststr\")\nNote(txt\"hello\")\nNote(md7\"text\")\n" $1
}

function test_converters {
    expect_pass "converters.ms" $1
    expect_out_eq "Title1
Paragraph1

Title2
Paragraph2
Hello
Foo1Class\n" $1
}

function test_generators {
    expect_pass_log "generators.ms continue" "-f html" $1
    expect_out_eq "<!DOCTYPE html>
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
</html>\n" $1

    expect_pass_log "generators.ms exit0" "-f html" $1
    expect_out_eq "<!DOCTYPE html>
<html>
<body>
<h1> Moss generated html</h1>
<p>Hello, World!</p>
</body>
</html>\n" $1

    # If exit with failure then we dont want to generate any output document as
    # it will be inclomplete anyway and there might be missing resources.
    expect_fail_log "generators.ms exit1" "-f html" "" $1
}

function test_converters_pipeline {
    expect_pass_log "converters_pipeline.ms" "-f D" $1
    expect_out_eq "A2B:B2C:C2D:hi
caught
B2C:C2D:FromB
A22B2:B22D:txt
C22D:txt2
A32B3:B32D:txt3\n" $1
}

function test_strings {
    expect_pass "strings.ms" $1
    expect_out_eq '\\n\\tsdf\\0x12\\n
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
' $1

    expect_fail_exec '"\xA"' "Short hexadecimal" $1
    expect_fail_exec '"\Q01"' "Short octal" $1
    expect_fail_exec '"\uA32"' "Short 16-bit unicode" $1
    expect_fail_exec '"\UA32845"' "Short 32-bit unicode" $1

    expect_fail_exec '"\xAG"' "Incorrect hexadecimal" $1
    expect_fail_exec '"\q080"' "Incorrect octal" $1
    expect_fail_exec '"\u08Y3"' "Incorrect 16-bit unicode" $1
    expect_fail_exec '"\U081336AZ"' "Incorrect 32-bit unicode" $1
}

function test_fstrings {
    expect_pass "fstrings.ms" $1
    expect_out_eq "v: 43 end
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
lorem ipsum.\n" $1
}

function test_docstrings {
    expect_pass "docstrings.ms" $1
    expect_out_eq "Module info
Second line
Returns 42 Second comment.
Class ACls
Constructor
Some space\n" $1
}

function test_multivar {
    expect_pass "multivar.ms" $1
    expect_out_eq "1 2 3 4
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
caught\n" $1
}

function test_range_precedence {
    expect_pass "range_precedence.ms" $1
    expect_out_eq "[[1], [3]]
[[1, 2], [3], 5, <object of class Range>]
[1, <object of class Range>]
[1, 1]
[1, \"two\", true, [8, 9], []]
6
7
[1, 3, <object of class Range>]
[<object of class Range>]
[[1, <object of class Range>]]
[[<object of class Range>]]\n" $1
}

function test_short_circuit_eval {
    expect_pass "short_circuit_eval.ms" $1
    expect_out_eq "true\ntrue\nhi\nthere\nfalse\nfail\nexists!\ndoes not exist.\n" $1
}

function test_unpacking {
    expect_pass "unpacking.ms" $1
    expect_out_eq "1 true hi
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
[1, 2, 3, \"hi\", [1, 2]]\n[0, 1, 2, 3, 4]\n[0, 1, 2, 3, 4]\n" $1
}

function test_equalities {
    expect_pass "equalities.ms" $1
    expect_out_eq "true\nfalse\ntrue\nfalse\nfalse\ncaught\nfalse\ntrue\ncaught
false\ntrue\ntrue\nfalse\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\n" $1
}

function test_memberships {
    expect_pass "memberships.ms" $1
    expect_out_eq "false\ntrue\nfalse\ncaught\ntrue\nfalse\ntrue\nfalse\ntrue
false\ntrue\ntrue\ntrue\nfalse\nfalse\ntrue\nfalse\ntrue\n" $1
}

function test_escaping {
    expect_pass "escaping.ms" $1
    expect_file_eq $OUTP_STD "${TEST_DIR}escaping_expected.txt" $1
}

function test_enable_code_output {
    expect_pass_log "enable_code_output.ms" "-f md" $1
    expect_file_eq $OUTP_STD "${TEST_DIR}enable_code_output_expected.md" $1
}

function test_static_methods {
    expect_pass "static_methods.ms" $1
    expect_out_eq "caught\nid, 6\ncaught\n" $1
}

function test_type_casting {
    expect_pass "type_casting.ms" $1
    expect_out_eq "42\ntrue\n42\n42.000000\ncaught\ncaught\ncaught\n" $1
}

function test_warnings {
    local out_exp="start
-9223372036854775807
9223372036854775807
inf
-0.000000
-inf\n"
    # no -W means -W ignore
    expect_pass "warnings.ms" $1
    expect_err_eq "" $1
    expect_out_eq "$out_exp" $1
    expect_pass_log "warnings.ms" "-Wi" $1
    expect_err_eq "" $1
    expect_out_eq "$out_exp" $1

    # -W error has to fail
    expect_fail_log "warnings.ms" "-W error" "error:" $1

    # -W all outputs errors to stderr and continues
    expect_pass_log "warnings.ms" "-W all" $1
    # pass_log will concat the streams, but beware that the highlights will be
    # offset and so for a new version it is better to copy $OUTP_STD or
    # just create single tests for this one warning
    expect_file_eq $OUTP_STD "${TEST_DIR}warnings_expected.txt" $1
}

function test_basic_import {
    expect_pass_compile "module_tests/greet_bc.ms" "module_tests/greet_compiled.msb" $1
    expect_pass "module_tests/module.ms" $1
    expect_out_eq "module.ms started
Hello, from greet.ms
Back in module
Hi, from msb greet
<module greet>
<module greet_compiled>
Hello, from greet.ms
greet's name: greet.ms (greet.ms)
Hello, from greet.ms
greet.ms
Ending module.ms\n" $1

    # Check that import spill stays in its scope
    expect_fail "module_tests/local_import.ms" "'NAME' is not defined" $1
    expect_out_eq "Hello, from greet.ms\ngreet.ms\n" $1

    rm -f ${TEST_DIR}module_tests/greet_compiled.msb
}

function test_import_calls {
    expect_pass "module_tests/square.ms" $1
    expect_out_eq "mod1 ran!\nanon_space_value\nsquare\nmod2fun\ngot result\n25\n9\n100\n49
mod1 ran!\nanon_space_value\nsquare_all\nmod2fun\ngot result\n16\n6\nfalse\ninner_fun 1\n6\n9\ncaught\ncaught\n" $1
}

function test_space_imports {
    expect_pass "space_imports.ms" $1
    expect_out_eq "caught\n1-msf\ncaught\nval1\n2-msf\nval1\n3-msf\nlocal val1
val1\ninner local val1\nlocal val1\nval1\n" $1
}

function test_closures {
    expect_pass "closures.ms" $1
    expect_out_eq "24\nOC; Created Inner + OC; <object of class InnerClass>\n<class InnerClass>
<b><i>Hi there!</i></b>\ntriple_val\ndouble_val\n" $1
}

function test_implicit_calls {
    expect_pass "implicit_calls.ms" $1
    expect_out_eq "hello is my string\nbye is my string\n42\n<object of class MyNumber>
AAA - [1, 2, 3]\n" $1
}

function test_fibonacci {
    expect_pass "fibonacci.ms" $1
    expect_out_eq "1\n55\n233\n2584\n" $1
}

function test_factorial {
    expect_pass "factorial.ms" $1
    expect_out_eq "2432902008176640000" $1
}

function test_collatz {
    expect_pass "collatz.ms" $1
    expect_out_eq "21 64 32 16 8 4 2 1 
2097152 1048576 524288 262144 131072 65536 32768 16384 8192 4096 2048 1024 512 256 128 64 32 16 8 4 2 1 
22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 " $1
}

function test_factorial {
    expect_pass "factorial.ms" $1
    expect_out_eq "2432902008176640000" $1
}

function test_pso {
    expect_pass "pso.ms" $1
}

function test_ascending_primes {
    expect_pass "ascending_primes.ms" $1
    expect_out_eq "[2, 3, 5, 7, 13, 17, 19, 23, 29, 37, 47, 59, 67, 79, 89, 127, 137, 139, 149, 157, 167, 179, 239, 257, 269, 347, 349, 359, 367, 379, 389, 457, 467, 479, 569, 1237, 1249, 1259, 1279, 1289, 1367, 1459, 1489, 1567, 1579, 1789, 2347, 2357, 2389, 2459, 2467, 2579, 2689, 2789, 3457, 3467, 3469, 4567, 4679, 4789, 5689, 12347, 12379, 12457, 12479, 12569, 12589, 12689, 13457, 13469, 13567, 13679, 13789, 15679, 23459, 23567, 23689, 23789, 25679, 34589, 34679, 123457, 123479, 124567, 124679, 125789, 134789, 145679, 234589, 235679, 235789, 245789, 345679, 345689, 1234789, 1235789, 1245689, 1456789, 12356789, 23456789]" $1
}

function test_lib_moss_module {
    expect_pass "stdlib_tests/moss_module.ms" $1
    expect_out_eq "<module libms>\n<space Math>\n3\ntrue\n" $1
}

function test_lib_constants {
    expect_pass "stdlib_tests/constants.ms" $1
    expect_out_eq_rx "[0-9]\.[0-9]\.[0-9]\n" $1
}

function test_lib_exit {
    expect_pass "stdlib_tests/exit.ms" $1
    expect_out_eq "caught 42\nhi\n" $1

    expect_fail_exec "exit(\"bye\")" "bye" $1
    expect_fail_exec "exit(42)" "" $1
    expect_retcode 42 $1

    expect_fail_exec "
fun exit(a, b) {
    a ++ \"\n\"
    exit(b)
}
exit(\"la fin\", 2)" "" $1
    expect_out_eq "la fin\n" $1
    expect_retcode 2 $1

    expect_fail "module_tests/exit_external.ms" "" $1
    expect_out_eq "started\n" $1
    expect_retcode 5 $1
}

# Note: This test is sensitive to bytecodegen changes because it uses
# hardcoded addresses
function test_lib_vardump {
    expect_pass "stdlib_tests/vardump.ms" $1
    expect_out_eq 'Int(42)
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
}\n' $1
}

function test_lib_print {
    expect_pass "stdlib_tests/print.ms" $1
    expect_out_eq '1 2 3 4\nhi-there.\n\ntrue\n1,2,3!\n' $1
}

function test_lib_type_constructors {
    expect_pass "stdlib_tests/type_constructors.ms" $1
    expect_out_eq '56\n56\n42\n10\n0\n22\n100\n0\n-8
0.000000\n2.500000\n50.000000\n15.000000
false\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue
Hi\n55\n<class String>\nnil
nil
[]\n[1, 2, 3]\n[true, nil, 2, 4]\n["H", "e", "l", "l", "o"]
[5, 6, 7, 8]\n[5, 6, 7, 8]\n[]
[0, 1, 2, 3, 4, 5]\n[0, 1, 2, 3, 4, 5]\n[]
Exception: Some exception\n' $1
}

function test_lib_builtin_exceptions {
    expect_pass "stdlib_tests/builtin_exceptions.ms" $1
    expect_out_eq "Caught: Exception: 
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
Caught: FileNotFoundError: \n" $1
}

function test_lib_ranges {
    expect_pass "stdlib_tests/ranges.ms" $1
    expect_out_eq '1\n3\n5\n100\n99\n98\n-2\n3\n8\n0\n1\n2
0\n1\n2\nend\nend\nend\n0\n1\n2\n0\n-1\n-2\n' $1
}

function test_lib_input {
    expect_pass_redir "stdlib_tests/input.ms" "stdlib_tests/input.ms" $1 
    expect_out_eq "> // input function test\n// this file is also read\n5//\n" $1

    expect_pass_redir "stdlib_tests/input_eof.ms" "empty.ms" $1 
    expect_out_eq "caught eof\n" $1
}

function test_lib_lists {
    expect_pass "stdlib_tests/lists.ms" $1 
    expect_out_eq "2\n0\n8\ncaught
[1, 2, 3]\n[1, 2, 3, 4, [true, nil]]\n[1, 2, 3, 4, [true, nil], 9, [], true]
[1, true, 2, false]\nfalse\ntrue\n[1, 2]\n[1, 2, 3, 4, 5, 6]\n5\n3\n[1, 2, 4, 6]
[4, 3, 2, 1]\n[]\n[false, true]\n[3, 2, 1]
[1, 2, 3]\n1\n" $1
}

function test_lib_strings {
    expect_pass "stdlib_tests/strings.ms" $1 
    expect_out_eq "3\n12\n11
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
abc\na\n" $1
}

function test_lib_random {
    expect_pass "stdlib_tests/random.ms" $1 
    expect_out_eq "" $1
}

function test_lib_math {
    expect_pass "stdlib_tests/math.ms" $1 
    expect_out_eq "1\n5\n0\n8
0.00\n0.50\n0.00\n1.00\n0.00\n-0.9
1.00\n0.50\n0.00\n-1.0\n1.00\n-0.3
0.00\n1.00\n-0.0\n0.00\n-0.0\n2.29
3\n4\n0.000000\n-100.000000\n12\n7.712600\n-0.120000
1\n123.450000\n0\n0.000000\n42\ninf\ninf\nnan\nnan
10\n5\n0\ninf\n90
16\n-128\n12\n0
3.000000\n10\n-3\n0\n-2
[2, 1]\n[59, 22]\ncaught\n2.000000\n0.800000\n[0.000000, 2.300000]\n" $1
}

function test_lib_file {
    local DATA_FILE=.data_file_read.txt
    printf "Czechia\nSlovakia\n\nFrance\nEngland\nGermany\nItaly" > $DATA_FILE
    expect_pass "stdlib_tests/file.ms" $1 
    expect_out_eq "caught\ncaught\n<C++ value of type std::fstream>
[\"Czechia\", \"Slovakia\", \"\", \"France\", \"England\", \"Germany\", \"Italy\"]
written line
Lorem Ipsum
Lorem Ipsum\n" $1
    rm -f $DATA_FILE
    rm -r ".created_file_t.txt1" ".created_file_2.txt"
}

function test_lib_args {
    expect_pass "stdlib_tests/args.ms 1 2" $1
    expect_out_eq "[\"1\", \"2\"]" $1

    expect_pass "stdlib_tests/args.ms" $1
    expect_out_eq "[]" $1

    expect_pass "stdlib_tests/args.ms true someval ~ + -" $1
    expect_out_eq "[\"true\", \"someval\", \"~\", \"+\", \"-\"]" $1
}

function test_lib_types {
    expect_pass "stdlib_tests/types.ms" $1
    expect_out_eq "42\ntrue\ntrue\ncaught\ncaught
<class Int>\ntrue\ntrue\n<class Space>\n<class Type>\n<class Note>\n<class Type>
true\ntrue\ntrue\ntrue\ntrue\nfalse\nfalse\nfalse\ntrue
false\ntrue\nfalse\ntrue\nfalse\ntrue\ntrue\n" $1
}

function test_lib_complex {
    expect_pass "stdlib_tests/complex.ms" $1
    expect_out_eq "1-2i\n1,-2\ntrue\n5+0i\nfalse\nfalse\ntrue
-1.500000+3.200000i\n6-2i\n-0.500000+1.200000i\n-4-2i\n5-10i
true\n0.400000-2.300000i\ncaught\n-1+2i\ntrue\nfalse\nfalse\ntrue
0+0i\n0-2i\n15.000000+0i\n8+0i\n18+27i\n0+0i\n5.000000-1.500000i\ncaught
4+6i\n3-4i\n23+2i\n1.000000+2.000000i\n8+0i\n3-1i\n2-2i\n3.000000+4.000000i\n" $1
}

function test_lib_filters {
    expect_pass "stdlib_tests/filters.ms" $1
    expect_out_eq "true\nfalse\ntrue\nfalse\ntrue\nfalse\ntrue
false\ntrue\nfalse\ntrue\nfalse\ntrue\nfalse
4\nawesome\ncaught\nhi\n9\nnil\ncaught\nhi\n1\nnil\n" $1
}

function test_lib_attrs {
    expect_pass "stdlib_tests/attrs.ms" $1
    expect_out_eq '["MyValue", "__Bool", "__Float", "__Int", "__String", "foo", "x"]
["MyValue", "__Bool", "__Float", "__Int", "__String", "foo"]
["NAME", "TheirValue"]
[]
["MyValue", "SomeClas", "TheirValue", "moss", "mv", "tv"]
true, true, false, true, false, true, true, true
a, 42, scf, caught
-7, scf, scf, caught
caught, caught, caught, caught
' $1
}

function test_lib_enumerate {
    expect_pass "stdlib_tests/enumerate.ms" $1
    expect_out_eq "0. 1\n1. 2\n2. 3\n[0, 1][1, 2][2, 3]\n[100, 9][101, 8][102, 7][103, 6][104, 5]\n" $1
}

function test_lib_subprocess_system_and_run {
    expect_pass "stdlib_tests/subprocess_tests/system_and_run.ms" $1
    # Note: Since the output might shuffle we need to test the outputs
    # inside of the test
    expect_out_eq "hi!\nno_cap\n" $1
}


function test_gc_local_vars {
    expect_pass_log "gc_tests/local_vars.ms" "--v5=gc.cpp::sweep" "--stress-test-gc" $1
    expect_out_eq "gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: ENUM(MyEnum)
gc.cpp::sweep: Deleting: ENUM_VALUE(A)
gc.cpp::sweep: Deleting: ENUM_VALUE(B)
gc.cpp::sweep: Deleting: ENUM_VALUE(C)
gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: LIST(List)
done\n" $1
}

function test_gc_recursion {
    # This test just checks that GC is triggered without any special mode
    # If this fails it might be because of changes in gc threshold (next_gc) 
    expect_pass_log "gc_tests/recursion.ms" "--v1=gc.cpp::collect_garbage" $1
    # This cannot check output when --stress-test-gc is set
    if ! [[ "$RUN_TEST_FLAGS" =~ "--stress-test-gc" ]] ; then
        expect_out_eq "gc.cpp::collect_garbage: Running GC
gc.cpp::collect_garbage: Finished GC
377\n" $1
    fi
}

function test_gc_global_dependency {
    expect_pass_log "gc_tests/global_dependency.ms" "--v5=gc.cpp::sweep" "--stress-test-gc" $1
    expect_out_eq "gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
gc.cpp::sweep: Deleting frame
done\n" $1
}

function test_bc_read_write {
    # Run and generate bc_read_write.msb
    expect_pass_log "bc_rw_output.ms" "-o ${TEST_DIR}/bc_read_write" $1
    expect_out_eq "Hello, World!\n" $1

    expect_pass "bc_read_write.msb" $1
    expect_out_eq "Hello, World!\n" $1

    # Cannot output bc for input bc
    expect_fail_log "bc_read_write.msb" "-o test.msb" "Trying to dump bytecode for bytecode input" $1

    # Check header
    expect_pass_log "bc_read_write.msb" "--print-bc-header" $1
    expect_out_eq_rx "Bytecode header:\n  id:        0xff00002a\n  checksum:  0x0\n  version:   \(0x[0-9A-Za-z]*\) [0-9]+\.[0-9]+\.[0-9]+\n  timestamp: \([0-9]+\) .*\n" $1

    # Textual output
    expect_pass_log "bc_rw_output.ms" "-o ${TEST_DIR}/bc_read_write.txt" "-S" $1
    expect_out_eq "Hello, World!\n" $1
    expect_file_eq "${TEST_DIR}/bc_read_write_expected.txt" "${TEST_DIR}/bc_read_write.txt" $1

    rm -f ${TEST_DIR}/bc_read_write.msb
    rm -f ${TEST_DIR}/bc_read_write.txt
}

function test_note_options {
    # Normal run
    expect_pass "simple_notes.ms" $1
    expect_out_eq "Start\n# Title1\nParagraph1\n\n# Title2\nParagraph2\nEnd\n" $1

    # Quiet run without notes
    expect_pass_log "simple_notes.ms" "-q" $1
    expect_out_eq "Start\nEnd\n" $1
    expect_pass_log "simple_notes.ms" "--disable-notes" $1
    expect_out_eq "Start\nEnd\n" $1

    # -O option
    local OUTF="${TEST_DIR}/._note_opts_out.md"
    rm -f $OUTF
    expect_pass_log "simple_notes.ms" "-O ${OUTF}" $1
    expect_out_eq "Start\nEnd\n" $1
    expect_file_eq_str "# Title1\nParagraph1\n\n# Title2\nParagraph2\n" $OUTF $1

    # -O and -p
    rm $OUTF
    expect_pass_log "simple_notes.ms" "-O ${OUTF} -p" $1
    expect_out_eq "Start\n# Title1\nParagraph1\n\n# Title2\nParagraph2\nEnd\n" $1
    expect_file_eq_str "# Title1\nParagraph1\n\n# Title2\nParagraph2\n" $OUTF $1

    # Invalid combinations
    expect_fail_log "simple_notes.ms" "-q -O ${OUTF}" "Invalid combination of arguments" $1
    expect_fail_log "simple_notes.ms" "-q -p" "Invalid combination of arguments" $1
    
    rm $OUTF
}

function test_exceptions_catch {
    expect_pass "exceptions_catch.ms" $1
    expect_out_eq "NameError: a\nNameError: foo()\nModule not found
Assertion error\nType error\nAttribute error\nName error
Division by zero error\nFloat division by zero error\nDBZ\nFDBZ
Attribute error\nAttribute error\n" $1

    expect_fail_exec "some_name" "Name 'some_name' is not defined" $1
    expect_retcode 1 $1
}

function test_runtime_errors {
    expect_pass "runtime_errors.ms" $1
    expect_out_eq "OK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\n" $1
}

function test_repl_output {
    expect_pass_repl "repl_output.ms" $1
    expect_out_eq_rx ".*moss> moss> moss> 5\nmoss> hi\n.*\n.*\n.*<object of class Cls1>\nmoss> 5\nmoss> moss> moss> moss> moss> moss>" $1
}

function test_repl_exceptions {
    expect_pass_repl "repl_exceptions.ms" $1
    expect_err_eq_rx "Stacktrace:\n  top-level.*\n.*Incorrect call.*could not match arguments.*\nStacktrace:\n  foo\(a\) at <repl>\n  top-level scope.*\n.*Name 'b' is not defined.*\nStacktrace:\n  top-level scope at <repl>\n.*Incorrect call to function 'foo'.*passed more arguments.*\n" $1
}

###--- Running tests ---###

function run_all_tests {
    run_test empty
    run_test bc_read_write
    run_test note_options

    run_test output
    run_test expressions
    run_test variables
    run_test functions
    run_test ifs
    run_test whiles
    run_test switches
    run_test fors
    run_test try_catch
    run_test classes
    run_test attributes
    run_test subscript_set
    run_test inheritance
    run_test operator_funs
    run_test lists
    run_test list_comprehension
    run_test dicts
    run_test enums
    run_test space
    run_test optional_typing
    run_test indexing
    run_test range_expr
    run_test compound_assignment
    run_test calls
    run_test continues_and_breaks
    run_test scopes
    run_test lambdas
    run_test supers
    run_test notes
    run_test converters
    run_test converters_pipeline
    run_test generators
    run_test strings
    run_test fstrings
    run_test docstrings
    run_test multivar
    run_test function_overwriting

    run_test basic_import
    run_test import_calls
    run_test space_imports
    run_test closures
    run_test implicit_calls
    run_test exceptions_catch
    run_test runtime_errors
    run_test range_precedence
    run_test short_circuit_eval
    run_test unpacking
    run_test equalities
    run_test memberships
    run_test escaping
    run_test enable_code_output
    run_test static_methods
    run_test type_casting
    run_test warnings

    run_test fibonacci
    run_test factorial
    run_test collatz
    run_test pso
    run_test ascending_primes

    # stdlib tests
    run_test lib_moss_module
    run_test lib_constants
    run_test lib_exit
    run_test lib_vardump
    run_test lib_print
    run_test lib_type_constructors
    run_test lib_builtin_exceptions
    run_test lib_ranges
    run_test lib_input
    run_test lib_lists
    run_test lib_strings
    run_test lib_random
    run_test lib_math
    run_test lib_file
    run_test lib_args
    run_test lib_types
    run_test lib_complex
    run_test lib_filters
    run_test lib_attrs
    run_test lib_enumerate

    run_test lib_subprocess_system_and_run

    # gc tests
    run_test gc_local_vars
    run_test gc_recursion
    run_test gc_global_dependency

    #repl tests
    run_test repl_output
    run_test repl_exceptions
}

# Count all functions starting with test_ 
TEST_AMOUNT=$(declare -F | grep "test_" | wc -l)
if [ ! -z "$RUN_TEST_FLAGS" ]; then
    printf "${C_GRAY}Running all tests with \"${RUN_TEST_FLAGS}\"${C_OFF}\n"
fi
start_time=`date +%s`
run_all_tests
if [ ! -z "$FAILED_TESTS" -a "$FAILED_TESTS" != " " ]; then
    UNQ_TST=""
    failedam=0
    for i in $FAILED_TESTS; do
        if ! [[ ${UNQ_TST} =~ $i ]]
        then
            UNQ_TST+=" $i"
            ((++failedam))
        fi
    done
    printf "${C_RED}FAILED${C_OFF}: ${failedam} test(s) did not pass\n"
    printf "Failed tests:\n"
    for t in $UNQ_TST; do
        printf "\t${t}\n"
    done
else
    end_time=`date +%s`
    runtime=$((end_time-start_time))
    printf "${C_GREEN}SUCCESS${C_OFF}: $INDEX tests passed (after $runtime s)\n"
fi
