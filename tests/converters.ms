@formatter
fun md4(x) = x

@converter("md4", "txt")
fun md4_to_txt(n) {
    txt = ""
    for (c: n) {
        if (c != "#")
            txt ++= c
    }
    return txt
}

md4"""#Title1
Paragraph1

#Title2
Paragraph2
"""

fun foo(x) {
    @!formatter
    return x
}

fun foo2txt(x) {
    @!converter("foo", "txt")
    return x++"\n"
}

foo"Hello"

class Foo1 {
    fun __String() {
        return "Foo1Class\n"
    }
}

@formatter
fun md1(x) {
    return Foo1()
}

fun md12txt(n) {
    @!converter("md1", "txt")
    return n
}

md1"this will not show"