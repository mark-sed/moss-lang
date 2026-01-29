class Mod1Cls {
    fun foo() {
        "YEP"
        return Mod1Cls
    }
}

space ModSpace {
    NAME = "ModSpace\n"

    fun get_name() {
        return "::"++NAME
    }
}

fun mod1fun() {
    fun innerf2() {
        return "hi "
    }
    fun innerf() {
        innerf2()
        return 42
    }
    return innerf
}

fun get_list() {
    return [mod1fun, {"foo": Mod1Cls}]
}

fun get_mod2() {
    import external_module2 as e2
    return e2
}