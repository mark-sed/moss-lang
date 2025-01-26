fun mod2fun(a) {
    "mod2fun\n"
    return a*a
}

fun quiet_mod2fun(a) {
    return a*a
}

fun const_val(v) {
    if (v)
        return 6
    else
        return v
}

fun exit_mod3(code) {
    import mod3
    mod3.exit_me(code)
}

class Sum {
    fun Sum(x) {
        this.x = x
    }

    fun sum(a, b) {
        return a + b + this.x
    }
}