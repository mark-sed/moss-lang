#!/usr/bin/env bash

# Script for running moss tests and checking the results

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
    test_"$1"
    failed_now=${#FAILED_TESTS}
    if [[ $failed_now -eq $failed_am ]]; then
        printf "$(testnum) ${1}: ${C_GREEN}OK${C_OFF}\n"
    else
        printf "$(testnum) ${1}: ${C_RED}FAILED${C_OFF}\n"
    fi
}

###--- TESTS ---###

function test_empty {
    expect_pass "empty.ms" "empty"
    expect_out_eq "" "empty"
}

function test_output {
    expect_pass "output.ms" "output"
    expect_out_eq "42, true, false\nmoss language\n13\n9\n42\n" "output"
}

function test_expressions {
    expect_pass "expressions.ms" "expressions"
    expect_out_eq "27\n13\n261\ntrue\ntrue\nfalse\nfalse\ntrue\ntrue
true\nfalse\n9\n255\n0\n6699\n" "expressions"
}

function test_variables {
    expect_pass "variables.ms" "variables"
    expect_out_eq "42\n42\n44\n44\n5\nMarek (me)\n25\n50\n20\n2\n8\n3
82\n:herb:🌿❗\n" "variables"
}

function test_functions {
    expect_pass "functions.ms" "functions"
    expect_out_eq "hi there\n" "functions"
}

###--- Running tests ---###

function run_all_tests {
    run_test empty
    run_test output
    run_test expressions
    run_test variables
    run_test functions
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
