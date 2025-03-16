"hi "
// Lets make sure that we don't execute the function body
fun foo() {
    "not here\n"
}
"there\n"
foo()
"\n"

fun sum(a:Int, b:Int=3) {
    return a+b
}

sum(4, 5)++"\n"

fun overloading(a:Int, b:[Bool, Int]) {
    return a ++ b ++ "\n"
}

fun overloading(a:String, b:[Int]) {
    return a ++ b ++ "\n"
}

overloading(1, 1)
overloading(1, false)
overloading("test", 2)

sum = 42
sum ++ "\n"

fun fooa(x, y:Bool, z, q, ...c, d=1) {
    x ++ y ++ z ++ q ++ c ++ d
}

fun fooa(a, b, c=5) {
    a ++ b ++ c
}

fun fooa(a=0) {
    a
}

~fooa()
"\n"
~fooa(1)
"\n"
~fooa(1, 2, 3)
"\n"
~fooa(b=2, a=1)
"\n"
~fooa(b=2, a=1, c=3)
"\n"
~fooa(1, true, "a", nil)
"\n"
// Test string output
fooa ++ "\n"

fun vararg(a, b, ... c, d=false, e=97) {
    return a ++ b ++ c ++ d ++ e ++ "\n"
}

vararg(1, 2, 3, 4, 5, d=6, e=7)
vararg(1, 2, 3, 4, 5, e=7, d=6)
vararg(1, 2)
vararg(1, 2, 3, 4, 5)

fun vararg2(... a) {
    return a
}

vararg2()
"\n"
vararg2(1, 2, 3, 4)
"\n"

fun varargsf(a:[Int,Bool], ... b) {
    a ++ b ++ "\n"
}

~varargsf(1, 2)
~varargsf(1, 2, 3, 4)
~varargsf(0, 1, "ok", false, nil)
~varargsf(true, 1)

fun getID(x:String) {
    return 1
}

fun getID(x:[Int, Bool]) {
    return 0
}

fun getID(x) {
    return 42
}

getID("hi")
"\n"
getID(3)
"\n"
getID(true)
"\n"
getID(nil)
"\n"

fun greet() {
    return "hello from greet"
}

fun caller(a) {
    return a()
}

caller(greet)
"\n"

fun outterf(a) {
    fun innerf() {
        return "goo " ++ a
    }

    return innerf()
}

outterf("inner fun\n")

// We allow for types to be inside of variables and
// even expressions

types = [Int, String, Bool]

fun vared_type(a:types[0]) {
    return a + 4
}

fun vared_type(a:types[1]) {
    return a ++ "!"
}

fun vared_type(a:types[2]) {
    return not a
}

vared_type(-2)
"\n"
vared_type("hi")
"\n"
vared_type(false)
"\n"

// Test that primitive types are passed by value
// and objects by reference
fun modifier(a) {
    a += 1
    return nil
}

fun modifier2(a) {
    a = true
    return nil
}

val_int = 5
val_int
"\n"
~modifier(val_int)
val_int
"\n"

~modifier2(val_int)
val_int
"\n"

fun modifier3(a) {
    a.x += 1
    return nil
}

class MyClass1 {
    fun MyClass1(x) {
        this.x = x
    }
}

mc1 = MyClass1(8)
mc1.x
"\n"
~modifier(mc1.x)
mc1.x
"\n"
~modifier2(mc1)
mc1
"\n"
~modifier3(mc1)
mc1.x
"\n"

enum EnumVals {
    FIRST, SECOND
}

fun modifier4(a) {
    a = EnumVals.SECOND
    return nil
}

env = EnumVals.FIRST
env
"\n"
~modifier2(env)
env
"\n"
~modifier4(env)
env
"\n"