space Space2Extend {
    VALUE = "global\n"
}

Space2Extend.VALUE

space Space2Extend {
    VALUE = "overwritten value\n"
}

Space2Extend.VALUE

space Space2Extend {
    fun foo() {
        return "new fun\n"
    }
}

Space2Extend.VALUE
Space2Extend.foo()

// Local definition does not extend global
fun some_fun() {
    space Space2Extend {
        VALUE = "local only value\n"
    }

    Space2Extend.VALUE
    ::Space2Extend.VALUE
}

~some_fun()
Space2Extend.VALUE
Space2Extend.foo()

// Inner function does extend the outter one (closure scope)
fun outter_fun() {
    space Space2Extend {
        VALUE = "outter fun\n"
    }
    Space2Extend.VALUE
    ::Space2Extend.VALUE
    Space2Extend != ::Space2Extend
    "\n"

    fun inner() {
        space Space2Extend {
            VALUE = "inner fun\n"
        }
        Space2Extend.VALUE
        ::Space2Extend.VALUE
        Space2Extend == $Space2Extend
        "\n"
    }

    ~inner()
    Space2Extend.VALUE
}

~outter_fun()
Space2Extend.VALUE
Space2Extend.foo()