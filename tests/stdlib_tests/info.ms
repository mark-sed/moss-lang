fun foo() {
    d"""
    Does foo things.
    Also might do baz things.
    """
}

fun foo2(a) {
    d"""
    Does foo2 things.
    Also might do baz2 things."""
}


info(foo)
info(foo2)

fun bar() {
    d"returns nil."
}

info(bar)

fun oo() {}

info(oo)

fun p() {
    d""
}

info(p)

fun q() {
    d"""

    """
}

info(q)

fun more_lines(a, b) {
    d"""


    This function 

    Does some formatting


    """
}

info(more_lines)

fun with_code(a, b) {
    d"""

    This function can be called like this:
        `with_code(3, 5)`
    There are many benefits to this, eg:
      - Is nice
      - is cool
      - awesome
    
    """
}

info(with_code)

fun no_nl() {
    d"""When doc start on the quote line
    then the indent will stay..."""
}

info(no_nl)