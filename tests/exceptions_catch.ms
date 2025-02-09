try {
    a
} catch (e:NameError) {
    "NameError: a\n"
}

try {
    foo()
} catch (e:NameError) {
    "NameError: foo()\n"
}

try {
    import someNoneExistentModule321
} catch (e: ModuleNotFoundError) {
    "Module not found\n"
}

try {
    assert(false, "Yep")
} catch (e: AssertionError) {
    "Assertion error\n";
}

fun foof(a:Int) {
    return 0
}

try {
    foof("hi")
} catch (e: TypeError) {
    "Type error\n"
}

aint = 5
try {
    aint.b
} catch (e:AttributeError) {
    "Attribute error\n"
}

try {
    aint.c
} catch (e:NameError) {
    "Name error\n"
}