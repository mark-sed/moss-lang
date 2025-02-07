// NameError
try a
catch (_:NameError) "OK\n"

// AttributeError (NameError)
try {
    b = 1
    b.foo
} catch (_:AttributeError) "OK\n"

// TypeError
try {
    a = "ads"
    if (a) {
        "in!\n"
    }
} catch (_:TypeError) "OK\n"

// Type error
try {
    "ad"()
} catch (_:TypeError) "OK\n"

// Type error
try {
    fun extra(a) {}
    fun extra(a, b) {}
    extra()
} catch (_:TypeError) "OK\n"

// Type error
try {
    fun single() {}
    single(1)
} catch (_:TypeError) "OK\n"

// TypeError
try {
    class Foo {}
    f = Foo()
    not f
} catch (_:TypeError) "OK\n"

// TypeError
try {
    f * 2
} catch (_:TypeError) "OK\n"

// NameError
try {
    fun foo(a:SomeType) {}
} catch (_:NameError) "OK\n"

// ModuleNotFoundError
try {
    import modfoo8
} catch (_:ModuleNotFoundError) "OK\n"

// TypeError
try {
    @internal_bind
    class ClsFoo {}
} catch (_:TypeError) "OK\n"

// NameError
try {
    @internal_bind("SomeFooClass")
    class ClsFoo {}
} catch (_:NameError) "OK\n"

// TypeError
try {
    a = 2
    @internal_bind("a")
    class ClasFoo {}
} catch (_:TypeError) "OK\n"

// TypeError
try {
    @internal_bind("Exception")
    fun foo() {}
} catch (_:TypeError) "OK\n"

// TypeError
try {
    2 ^ "sf"
} catch (_:TypeError) "OK\n"

// TypeError
try {
    a = nil
    b = 95
    a > b
} catch (_:TypeError) "OK\n"

// TypeError
try {
    a = "sfd"
    not a
} catch (_:TypeError) "OK\n"

// AssertionError
try {
    assert(false, "This should be asserted");
} catch (_:AssertionError) "OK\n"