import mod2

fun square(a) {
    "square\n"
    res = mod2.mod2fun(a)
    "got result\n"
    return res
}

fun quiet_square(a) {
    return mod2.quiet_mod2fun(a)
}

fun square_all(a) {
    import mod2.*
    "square_all\n"
    res = mod2fun(a)
    "got result\n"
    return res
}

"mod1 ran!\n"

space {
    mod1_val_an_sp = "anon_space_value\n"
}

mod1_val_an_sp