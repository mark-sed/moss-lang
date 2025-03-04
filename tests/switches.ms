fun test1(a) {
    switch(a) {
        case 1, 2: return 4
        case 3: return 0
        default: return -1
    }
}

test1(1)
test1(2)
test1(3)
test1(-4)
"\n"

switch("hi") {
    case "hello": "hello!\n"
    case "hi": "hi!\n"
}

b = true
switch(b) {}

switch(b) {
    case nil: "nil\n"
    case false: "false\n"
    default: "def\n"
}

fun test2(a) {
    switch(a) {
        case nil, "": return -1
        case 1, true, "1", 1.0: return 1
    }
    return 0
}

test2("")
test2(nil)
"\n"
test2(1)
test2("1")
test2(1.0)
test2(true)
"\n"
test2(5)
test2(Int)
test2("3")
"\n"

class FooClass {
    fun FooClass(id) {
        this.id = id
    }

    fun (==)(other:FooClass) {
        return this.id == other.id
    }
    fun (==)(other) {
        return false
    }
}

fun test3(a) {
    fc1 = FooClass(4)
    fc2 = FooClass(42)
    switch(a) {
        case FooClass(1): "1\n"
        case fc1, fc2: "fc1\n"
        case fc2: "fc2\n" //unrechable
        default: "none\n"
    }
}

~test3(FooClass(42))
~test3(FooClass(1))
~test3(FooClass(4))
~test3(FooClass(100))
~test3(nil)