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

sum
"\n"
sum ++ overloading ++ "\n" // Making sure that concat also converts correctly
// Let's override the function definition with variable
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

x = fooa()
"\n"
x = fooa(1)
"\n"
x = fooa(1, 2, 3)
"\n"
x = fooa(b=2, a=1)
"\n"
x = fooa(b=2, a=1, c=3)
"\n"
x = fooa(1, true, "a", nil)
"\n"

fun vararg(a, b, ... c, d=false, e=97) {
    return a ++ b ++ c ++ d ++ e ++ "\n"
}

vararg(1, 2, 3, 4, 5, d=6, e=7)
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

c = varargsf(1, 2)
c = varargsf(1, 2, 3, 4)
c = varargsf(0, 1, "ok", false, nil)
c = varargsf(true, 1)

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