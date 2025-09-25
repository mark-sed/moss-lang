fun test1() {}

fun test2() {
    h = 4
}

fun test3(c, d) {}

a = "a"
b = 3

globs = globals()
assert(globs == {"b": 3, "a": "a", "test1": test1, "test2": test2, "test3": test3, "moss": moss},
    "globals differ")