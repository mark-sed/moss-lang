fun foo_raise(ex) {
    try {
        raise ex
    } catch (e2:[NameError, Int, Bool]) {
        "Caught "++e2++"!\n"
    } catch (f2) {
        "other\n"
    }
}

try {
    "Success\n"
} catch (e1:[NameError, Int]) {
    "Caught "++e1++"!\n"
} catch (f1) {
    "in f\n"
} finally {
    "finally end\n"
}

try {
    raise NameError("Unkown name")
} catch (e2:[NameError, Int]) {
    "Caught "++e2++"!\n"
} catch (f2) {
    "in f\n"
} finally {
    "finally end\n"
}

~foo_raise(true)
~foo_raise(nil)

try {
    raise 3
} catch (e3:[NameError, Int]) {
    "Caught "++e3++"!\n"
} catch (f3) {
    "in f\n"
} finally {
    "finally end\n"
}

try {
    raise "str"
} catch (e4:[NameError, Int]) {
    "Caught "++e4++"!\n"
} catch (f4) {
    "in f\n"
} finally {
    "finally end\n"
}

import ext_module.mod_raise

try {
    mod_raise(8)
} catch (d) {
    "Caught: " ++ d ++ "\n"
}

@internal
fun foo_int() { "Should not be called\n"; }

try {
    foo_int()
} catch (e:NameError) {
    "foo_int is not internal\n"
}

fun foo_get_reg() {
    b = 5
    try
        raise "a"
    catch(c) {}
    a = 4
    b
    a
    c
}

~foo_get_reg()
"\n"

fun foof() {
    a = 55
    try {
        try {
            raise 76
        } catch (e) {
            "inner\n"
        }
    } catch (f) {
        "outter\n"
    }
}

~foof()
~foof()

fun foof2(x) {
    a = 55
    try {
        try {
            raise x
        } catch (e:Int) {
            "inner\n"
        }
    } catch (f:String) {
        "outter\n"
    }
}

~foof2("dsfs")
~foof2(34)
~foof2(7)
~foof2("")

fun retfn(c) {
    a = 55
    try {
        if(c)
            raise c
    } catch (e) {
        return a
    }
    return "end"
}

retfn(true)
"\n"
retfn(false)
"\n"
retfn(true)
"\n"

fun foorec(v, v2) {
    a = 55
    try {
        if(v == 2)
            raise v
        else if (v == 3)
            return foorec(2, v2)
        else if (v == 4)
            return foorec(v2, v2)
    } catch (e:Int) {
        if (v2 == 0)
            return foorec(9, 1)
        return a
    }
    return "end"
}

foorec(2, 1)
"\n"
foorec(2, 0)
"\n"
foorec(3, 1)
"\n"
foorec(3, 0)
"\n"
foorec(3, 0)
"\n"
foorec(4, 3)
"\n"

"module end\n"