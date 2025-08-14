fun outter() {
    a = 24
    fun inner() {
        return a
    }
    return inner
}

outter()() ++ "\n"

// Accessing values of enclosing class
class OutterClass {

    NAME = "OC"

    class InnerClass {
        fun InnerClass() {
            "Created Inner + " ++ NAME ++ OutterClass.delimiter(this)
        }
    }

    fun delimiter() {
        return "; "
    }

    fun get_inner() {
        NAME ++ this.delimiter()
        return this.InnerClass()
    }
}

oc = OutterClass()
oc.get_inner()
"\n"
OutterClass.InnerClass++"\n"

// Actual example
fun bold(fn) {
    fun wrapper() {
        return "<b>" ++ fn() ++ "</b>"
    }
    return wrapper
}

fun italic(fn) {
    fun wrapper() {
        return "<i>" ++ fn() ++ "</i>"
    }
    return wrapper
}

fun text() {
    return "Hi there!"
}

text_bold = bold(italic(text))
text_bold()++"\n"
//

fun triple() {
    fun double() {
        fun single() {
            return a ++ b
        }
        b = "double_val\n"
        return single
    }
    a = "triple_val\n"
    return double
}

triple()()()