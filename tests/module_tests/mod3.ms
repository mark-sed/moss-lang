fun outter_fun(a) {
    fun inner_fun(b) {
        "inner_fun " ++ b ++ "\n"
    }

    ~inner_fun(a)
}