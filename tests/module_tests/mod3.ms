fun outter_fun(a) {
    fun inner_fun(b) {
        "inner_fun " ++ b ++ "\n"
    }

    ~inner_fun(a)
}

fun sum3(a, b, c=0) {
    return a + b + c
}