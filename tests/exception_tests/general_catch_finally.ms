fun catch_order() {
    try {
        raise "hello"
    } catch (e) {
        "caught generic\n"
    } catch (s:String) {
        "caught string\n"
    } finally {
        "finally\n"
    }
}

~catch_order()
"---\n"

fun rethrow_test() {
    try {
        try {
            raise "boom"
        } catch (s:String) {
            "inner catch\n"
            raise s
        } finally {
            "inner finally\n"
        }
    } catch (e) {
        "outer catch\n"
    } finally {
        "outer finally\n"
    }
}

~rethrow_test()
"---\n"

fun finally_throws() {
    try {
        try {
            raise "orig"
        } catch (e) {
            "inner caught\n"
        } finally {
            "finally before throw\n"
            raise "from finally"
        }
    } catch (e) {
        "outer caught: "
        e
        "\n"
    } finally {
        "outer finally\n"
    }
}

~finally_throws()
"---\n"

fun return_vs_finally() {
    try {
        try {
            "in try\n"
            return
        } catch (e) {
            "should not happen\n"
        } finally {
            "finally runs?\n"
        }
    } catch (e) {
        "outer catch?\n"
    } finally {
        "outer finally runs?\n"
    }
}

~return_vs_finally()
"---\n"

fun return_in_finally() {
    try {
        try {
            "try block\n"
            return 0
        } catch (e) {
            "nope\n"
        } finally {
            "finally overrides\n"
            return 5
        }
    } catch (e) {
        "outer catch\n"
    } finally {
        "outer finally\n"
    }
}

return_in_finally()
"\n---\n"

fun break_in_catch() {
    i = 0
    while (i < 3) {
        try {
            if (i == 1) {
                raise "err"
            }
            "ok\n"
        } catch (e) {
            "caught\n"
            break
        } finally {
            "finally loop\n"
        }
        i = i + 1
    }
}

~break_in_catch()
"---\n"

fun deep_finally_order() {
    try {
        try {
            try {
                raise "x"
            } catch (e) {
                "inner caught\n"
                raise e
            } finally {
                "finally A\n"
            }
        } catch (e) {
            "middle caught\n"
            raise e
        } finally {
            "finally B\n"
        }
    } catch (e) {
        "outer caught\n"
    } finally {
        "finally C\n"
    }
}

~deep_finally_order()
"---\n"

fun dynamic_types(x) {
    try {
        if (x == 0) raise "str"
        if (x == 1) raise true
        if (x == 2) raise 3.14
        if (x == 3) raise nil
    } catch (e) {
        if (type(e) == String) {
            "string\n"
        } else if (type(e) == Bool) {
            "bool\n"
        } else if (type(e) == Float) {
            "float\n"
        } else if (type(e) == NilType) {
            "nil\n"
        } else {
            "unknown\n"
        }
    } finally {
        "finally\n"
    }
}

~dynamic_types(0)
~dynamic_types(1)
~dynamic_types(2)
~dynamic_types(3)
"---\n"

fun uncaught() {
    try {
        raise "no handler"
    } catch (e) {
        "inner caught, rethrow\n"
        raise e
    } finally {
        "finally still runs\n"
    }
}

try
    ~uncaught()
catch (e)
    "top caught\n"
finally
    "top finally\n"
"---\n"

space {
fun exception_in_catch() {
    fun inner() {
        class SomeClass {
            fun exc() {
                try {
                    raise "x"
                } catch (e) {
                    "in catch\n"
                    raise "from catch"
                } finally {
                    "finally inner\n"
                }
            }
        }
        sc = SomeClass()
        return sc.exc()
    }
    return inner()
}
}

try
    ~exception_in_catch()
catch (e)
    "outer caught\n"
finally
    "outer finally\n"
"---\n"

fun clean_path() {
    try {
        try {
            "no error\n"
        } catch (e) {
            "should not happen\n"
        } finally {
            "finally inner\n"
        }
    } catch (e) {
        "outer catch\n"
    } finally {
        "finally outer\n"
    }
}

~clean_path()