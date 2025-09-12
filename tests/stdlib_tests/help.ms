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

class MyCls {
    d"""
    It is my class.
    Does classy things.
    """
}

help(MyCls)

enum EnB {
    A, B, C, D, E
}

help(EnB)

enum EmptyEnum {}

help(EmptyEnum)

space SomeSpace {
    d"""
    Just some space being spaceous.
    Is tied to SomeTime.
    Might bend.
    """
}

help(SomeSpace)