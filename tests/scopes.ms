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

a = 1
try {
    $a
} catch (e:SyntaxError) {
    "caught\n"
}

fun f1() {
    a = 2
    $a
}

try {
    f1()
} catch (e:NameError) {
    "caught\n"
}

fun outter() {
    a = 2
    fun inner() {
        a = 3
        $a
    }
    ~inner()
}

~outter()
"\n"

fun fn1() {
    a = 2
    fun fn2() {
        a = 3
        fun fn3() {
            a = 4
            return $a ++ "\n"
        }
        return fn3()
    }
    return fn2()
}

fn1()