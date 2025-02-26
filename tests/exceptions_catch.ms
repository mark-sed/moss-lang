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

zero = 0
zerof = 0.0

try {
    42 / zero
} catch(e:DivisionByZeroError) {
    "Division by zero error\n"
}

try {
    5 / zerof
} catch(e:DivisionByZeroError) {
    // TODO: Extract the message that it says "Float division by zero"
    "Float division by zero error\n"
}

try {
    zero % zero
} catch(e:DivisionByZeroError) {
    "DBZ\n"
}

try {
    (zero + 5 * 2) % zerof
} catch(e:DivisionByZeroError) {
    "FDBZ\n"
}

try {
    a = 5
    a.v = true
} catch (e:AttributeError) {
    "Attribute error\n"
}

try ("string").length = 5; catch (e:AttributeError) {
    "Attribute error\n"
}