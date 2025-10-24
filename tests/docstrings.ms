d"""
Module info
Second line
"""
__doc.strip()
"\n"

fun foo() {
    d"""
    Returns 42 """
    d"Second comment."
    return 42
}

foo.__doc.strip()
"\n"

class ACls {
    /*
    Comment
    */
    d"Class ACls"

    fun ACls() {
        // Also comment
        d"""
        Constructor
        """
    }
}

ACls.__doc.strip()
"\n"
ACls.ACls.__doc.strip()
"\n"

space SomeSp {
    d"""
    Some space
    """
}

SomeSp.__doc.strip()
"\n"

fun foo2(a, b) {
    d"foo(a,b)"
}

fun foo2(a) {
    d"foo(a)"
}

fun foo2() {
    d"foo()"
}

for (f: foo2) {
    f.__doc++"\n"
}

dcs = info(foo2)
assert(type(dcs) == List, "info did not return list of docs")
assert(dcs.length() == 3, "Missing doc in doc list")
i = 0
for (f: foo2) {
    assert(f == dcs[i][0], f++" is missing in doc list")
    i += 1
}

fun foo3(a, b) {
    d"foo(a,b)"
}

fun foo3(a) {
}

fun foo3() {
    d"foo()"
}

dcs2 = info(foo3)
assert(type(dcs2) == List, "info did not return list of docs -- 2")
assert(dcs2.length() == 3, "Missing doc in doc list -- 2")
i = 0
for (f: foo3) {
    assert(f == dcs2[i][0], f++" is missing in doc list --2")
    i += 1
}

enum FooNum {
    d"""
    Foo enum.
    Has ABC.
    """
    A
    B
    C
}

dnenum = info(FooNum)
dnenum