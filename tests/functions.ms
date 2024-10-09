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