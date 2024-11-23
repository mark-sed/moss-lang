[1, 2, 3, not true, "pět"]
"\n"

fun foo() {
    return [123]
}

[(foo()), 4, foo()]
"\n"

fun bar(a:[List]) {
    a
    return a
}

val = 7
a = bar([5, 8, [0, -1, -val]])
"\n"
a
"\n"
