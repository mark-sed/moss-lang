fun foo(a, b) {
    d"Does foo(a,b)"
}

fun foo(a) {
}

fun foo() {
    d"Does foo()"
}

help(foo)

fun bar(a:Int, b=true) {
    d"""
    Will do bar.
    And return some val.

    """
}

help(bar)