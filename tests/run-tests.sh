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
true\nfalse\n9\n255\n0\n6699\n-42\nfalse\n" $1
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
1\n0\n0\n42\nhello from greet\ngoo inner fun\n" $1
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
    expect_out_eq "42\n<class Cat>\n<object of class Cat>
Vilda\nmeow\n<object of class Animal>\n" $1
}

function test_attributes {
    expect_pass "attributes.ms" $1
    expect_out_eq "constructed\n56\n56\n56\n<object of class Foo>
<object of class B>\n91\n91\n91 != 65\n-1 == -1\n" $1


}

function test_lists {
    expect_pass "lists.ms" $1
    expect_out_eq "[1, 2, 3, false, \"pět\"]\n[[123], 4, [123]]
[5, 8, [0, -1, -7]]\n[5, 8, [0, -1, -7]]\n" $1
}

function test_enums {
    expect_pass "enums.ms" $1
    expect_out_eq "<Enum Colors>\nEnum {
  Blue,
  Red,
  Green,
  Purple
}\n" $1
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

function test_lib_exit {
    expect_pass "stdlib_tests/exit.ms" $1
    expect_out_eq "hi\n" $1

    expect_fail_exec "exit(\"bye\")" "bye" $1
    expect_fail_exec "exit(42)" "" $1

    expect_fail_exec "
fun exit(a, b) {
    a ++ \"\n\"
    exit(b)
}
exit(\"la fin\", 2)" "" $1
    expect_out_eq "la fin\n" $1
}

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
]\nEnum {}\n' $1
}

function test_lib_print {
    expect_pass "stdlib_tests/print.ms" $1
    expect_out_eq '1 2 3 4\nhi-there.\n\ntrue\n1,2,3!\n' $1
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
    expect_out_eq "gc.cpp::collect_garbage: Running GC
gc.cpp::collect_garbage: Finished GC
233\n" $1
}

function test_gc_global_dependency {
    expect_pass_log "gc_tests/global_dependency.ms" "--v5=gc.cpp::sweep" "--stress-test-gc" $1
    expect_out_eq "gc.cpp::sweep: Deleting: LIST(List)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
gc.cpp::sweep: Deleting: INT(Int)
gc.cpp::sweep: Deleting: STRING(String)
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

###--- Running tests ---###

function run_all_tests {
    run_test empty
    run_test bc_read_write

    run_test output
    run_test expressions
    run_test variables
    run_test functions
    run_test ifs
    run_test whiles
    run_test classes
    run_test attributes
    run_test lists
    run_test enums

    run_test fibonacci
    run_test factorial
    run_test collatz

    # stdlib tests
    run_test lib_exit
    run_test lib_vardump
    run_test lib_print

    # gc tests
    run_test gc_local_vars
    run_test gc_recursion
    run_test gc_global_dependency

    # xfails
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
