[1, 2, 3, false]
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

a = bar([5, 8, [0, 1]])
"\n"
a
"\n"