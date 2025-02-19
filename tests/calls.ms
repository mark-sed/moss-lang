fun foo(a:Int) {
    return "called\n"
}

foo(4)
try {
    foo()
} catch (e) {
    e.msg
}

try {
    foo("hi")
} catch (e) {
    e.msg
}

try {
    foo(1, 2)
} catch (e) {
    e.msg
}

class Acls {

    fun say(i) {
        return "hi\n"
    }
}

a = Acls()
Acls.say(4, a)

try {
    Acls.say(1)
} catch (e) {
    e.msg
}

try {
    Acls.say(0)
} catch (e) {
    e.msg
}

try {
    Acls.say(0, 2, 5)
} catch (e) {
    e.msg
}