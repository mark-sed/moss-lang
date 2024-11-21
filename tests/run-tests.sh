#!/usr/bin/env bash

# Script for running moss tests and checking the results
# example run: bash tests/run-tests.sh -moss build/moss -test-dir tests/

MOSS="moss"
OUTP_ERR=/tmp/.moss_test_err.txt
OUTP_STD=/tmp/.moss_test_std.txt
TEST_DIR="" # Directory where the test are located
POSITIONAL_ARGS=()

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

function failed {
    FAILED_TESTS+=" $1"
    printf "${C_RED}FAIL${C_OFF}: $1: $2\n"
}

function run {
    CMD="$MOSS ${TEST_DIR}$1"
    $CMD 2>$OUTP_ERR 1>$OUTP_STD
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

function expect_out_eq {
    outstr=$(cat $OUTP_STD)
    if ! cmp  "$OUTP_STD" <(printf "$1") ; then
        failed $2 "Output differs"
        printf "Expected:\n-------\n${1}\n"
        printf "Got:\n----\n${outstr}\n"
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
true\nfalse\n9\n255\n0\n6699\n" $1
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
12[3, 4, 5]false97\n12[]false97\n12[3, 4, 5]false97
[]\n[1, 2, 3, 4]\n1[2]\n1[2, 3, 4]\n0[1, \"ok\", false, nil]\ntrue[1]
1\n0\n0\n42\nhello from greet\n" $1
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

function test_classes {
    expect_pass "classes.ms" $1
    expect_out_eq "42\n<Class Cat>\n" $1
}

function test_lists {
    expect_pass "lists.ms" $1
    expect_out_eq "[1, 2, 3, false, \"pět\"]\n[[123], 4, [123]]
[5, 8, [0, 1]]\n[5, 8, [0, 1]]\n" $1
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

function test_exit {
    expect_pass "stdlib_tests/exit.ms" $1
    expect_out_eq "hi\n" $1
}

function test_vardump {
    expect_pass "stdlib_tests/vardump.ms" $1
    expect_out_eq "Int(42)
String(\"hello there\")\nString(\"\")\nBool(true)\nNilType(nil)
List(4) [
  Int(1),
  String(\"two\"),
  Bool(true),
  List(2) [
  Int(8),
  Int(9)
]
]
List(0) []\n" $1
}

###--- Running tests ---###

function run_all_tests {
    run_test empty
    run_test output
    run_test expressions
    run_test variables
    run_test functions
    run_test ifs
    run_test whiles
    run_test classes
    run_test lists

    run_test fibonacci
    run_test factorial
    run_test collatz

    # stdlib tests
    run_test exit
    run_test vardump
}

# Count all functions starting with test_ 
TEST_AMOUNT=$(declare -F | grep "test_" | wc -l)
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
