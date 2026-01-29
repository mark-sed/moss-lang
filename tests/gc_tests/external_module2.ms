fun get_mod_spc_extended() {
    import external_module.*
    space ModSpace {
        NAME = "NewNameMod2\n"
        fun mod2fun() {
            return "mod2fun\n"
        }
    }
    return ModSpace
}

fun helper() {
    return "mod2\n"
}

fun get_name() { 
    return helper
}