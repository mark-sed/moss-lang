"module.ms started\n"

import greet

"Back in module\n"

import greet_compiled as grcmp

greet
"\n"
grcmp
"\n"

import greet.NAME as GsName

"greet's name: " ++ greet.NAME ++ " (" ++ GsName ++ ")\n"

fun foo() {
    import greet.*
    NAME ++ "\n"
}

~foo()

"Ending module.ms\n"