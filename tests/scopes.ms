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

globa_val = -1

fun outterf1(c) {
    a = 12
    a
    "\n"
    b = 5
    b
    "\n"
    fun inner() {
        ::globa_val = -5
        $a = "changed\n"
        $b = "also\n"
        try $d = 5
        catch (e:NameError) "caught\n"
    }
    ~inner()
    e = 24
    a
    b
    c
    "\n"
}

globa_val
"\n"
~outterf1(4)
globa_val
"\n"

a2 = 3
try
    ::b = 4
catch(e:NameError)
    "caught\n"
::a2
"\n"
::a2 = 1
::a2
"\n"

fun compound_non_loc() {
    a = "hi"
    b = 40

    fun f2() {
        a = "\n"
        b = 5
        $a ++= " there"
        $a ++= a
        $b ^= 0
        $b += 2
        $b -= 1
        $b /= 2 // 1
        $b *= b
        $b %= 3
    }

    ~f2()
    b
    "\n"
    return a
}

compound_non_loc()