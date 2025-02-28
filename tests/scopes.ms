a = 5
::a
"\n"

fun ret_glob() {
    a = 1
    return ::a + 1
}

ret_glob()
"\n"

class C {
    fun get_v() { return 2; }
}

c = C()
::c.get_v()
"\n"

fun ret_glob_c() {
    class C {}
    c = C()
    return ::C.get_v(::c)
}

ret_glob_c()
"\n"

~::print("hi")