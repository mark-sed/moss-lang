fun foo() {
    return "foo1\n"
}

fun foo() {
    return "foo2\n"
}

foo()

fun bar(a, b, c) {
    return "bar1\n"
}

fun bar(v, x, z) {
    return "bar2\n"
}

fun bar(o, p, q=3) {
    return "bar3\n"
}

bar(1, 2)
bar(1, 2, 3)

fun f() {
    return "f1\n"
}

fun f(a) {
    return "f2\n"
}

f()
f(1)

fun g(a) {
    return "ga\n"
}

fun g(a:Bool) {
    return "gabool\n"
}

fun g(a:Int, b, c:[Bool, String]) {
    return "g1\n"
}

fun g(a:Int, b, c:[String, Bool]) {
    return "g2\n"
}

g(1, 2, "")

fun g(a:Int, b, c:[String, Bool]) {
    return "g3\n"
}

g(1, 2, "")
g(4)


fun baz(a, b, ...c) {
    return "baz1\n"
}

fun baz(a, b, ...c) {
    return "baz2\n"
}

fun baz(a, b, ...c) {
    return "baz3\n"
}

baz(1, 2)
baz(1, 2, 3, 4)

fun h(a:[Int, String], b:[Int, Bool]) {
    return "h1\n"
}

fun h(a:[Int, String], b:[Int, Float]) {
    return "h2\n"
}

h(1, 2)
h(1, 1.2)

fun l(a, v, v) {}
fun l(a, v, v, j) {}

fun l(... a, b:Int=1) {
    return "l1\n"
}

fun l(... a, b:Int=3) {
    return "l2\n"
}

l()


class P {
    fun P(a:Int, b:[Int, Float], c=true) {
        ~print("P1")
    }

    fun P(a:Int, b:[Int, Float], c=true) {
        ~print("P2")
    }

    fun P(a:Int, b:[Int, Float], c, d) {
        ~print("P3")
    }

    fun P() { ~print("P4"); }
}

~P(3, 4.4)
~P(3, 4.4, "", "")
~P()
