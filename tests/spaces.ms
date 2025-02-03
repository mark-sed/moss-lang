space FooSpace {
    NAME = "FooSpace"
    "In "++ NAME ++"\n"

    fun get_name() {
        return NAME
    }
}

space BarSpace {
    NAME = "BarSpace"
    "In "++ NAME ++"\n"

    fun get_name() {
        return NAME
    }

    "I can see: " ++ FooSpace.NAME ++ "\n"
}

FooSpace.get_name()
"\n"
FooSpace.NAME
"\n"

BarSpace.get_name()
"\n"
BarSpace.NAME
"\n"


space Foo {
    NAME = "Foo"

    space {
        NAME ++ "\n"
    }

    space {
        NAME ++ "\n"
    }
}

space {
    "Anonymous space\n"
    Foo ++ "\n"
}
