fun test1() {}

fun test2() {
    h = 4
}

fun test3(c, d) {}

a = "a"
b = 3

globs = globals()
assert(globs == {"b": 3, "a": "a", "test1": test1, "test2": test2, "test3": test3,
                 "moss": moss, "__FILE": __FILE, "__NAME": __NAME},
    "globals differ")

import empty_module

if (__FILE[-1] == "b") {
    // When compiled this will be msb
    __FILE[__FILE.length()-11..__FILE.length()-1]
} else 
    __FILE[__FILE.length()-10..__FILE.length()]
"\n"

ef = empty_module.__FILE
ef[ef.length()-15..ef.length()]
"\n"

__NAME
"\n"
empty_module.__NAME
"\n"