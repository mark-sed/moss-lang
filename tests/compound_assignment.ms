class A {
    fun A() {
        this.i = "I say "
        this.v = 2
    }
}

a = A()
c = "hi"
a.i ++= c
a.i
"\n"
a.i ++= "!"
a.i
"\n"

class B {
    fun B(a) {
        this.a = a
    }
}

b = B(a)
b.a.i ++= "?\n"
a.i

a.v
"\n"
a.v ^= 2
a.v
"\n"

a.v += 10
a.v
"\n"

a.v -= 5
a.v
"\n"

a.v /= 3
a.v
"\n"

b.a.v *= 2
a.v
"\n"

a.v %= 4
a.v
"\n"