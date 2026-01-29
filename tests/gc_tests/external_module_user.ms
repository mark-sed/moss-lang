fun get_mod_obj(n) {
    import external_module
    return external_module.Mod1Cls()
}

get_mod_obj("").foo()
"\n"

fun get_mod_spc() {
    import external_module
    return external_module.ModSpace
}

get_mod_spc().NAME
get_mod_spc().get_name()

fun get_mod_spc_extended() {
    import external_module.*
    space ModSpace {
        NAME = "NewName\n"
    }
    return ModSpace
}

get_mod_spc_extended().NAME
get_mod_spc_extended().get_name()

fun get_mod_spc_3mods() {
    import external_module2
    return external_module2.get_mod_spc_extended()
}

get_mod_spc_3mods().NAME
get_mod_spc_3mods().get_name()
get_mod_spc_3mods().mod2fun()

fun get_mod_fun() {
    import external_module
    return external_module.mod1fun
}

get_mod_fun()()()
"\n"

fun get_mod_in_list() {
    import external_module
    return external_module.get_list()
}

l = get_mod_in_list()
l[0]()()
"\n"
l[1]["foo"].foo(l[1])
"\n"

fun get_mod() {
    import external_module
    return external_module.get_mod2()
}

get_mod().get_name()()