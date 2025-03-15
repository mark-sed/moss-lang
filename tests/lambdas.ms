fun foo(a:[Int, String]) = "foo: " ++ a ++ "\n"

foo(42)
foo("hi")
try foo(true)
catch(e:TypeError) "caught\n"

glob_val = 5

fun access(a) = glob_val + a
access(3)
"\n"

fun outter() = fun inner(a) = a ++ "-inner\n"
outter()(5)

fun outter1() {
    x = "outter"
    fun inner(x) = $x++"\n"
    return inner(56)
}

outter1()

(fun(x:[Float]) = Int(x)+1)(4.2)
"\n"

converter = fun(txt) = "> " ++ txt ++ ".\n"
converter("moss is great")

add = fun(x, y) = x + y
add(2, 3)
"\n"
add(-1, 1)
"\n"