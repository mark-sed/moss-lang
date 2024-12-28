class A {
    fun A() {
        "A"
    }

    fun a_call() {
        "a_call\n"
    }
}

class B : A {
    fun B() {
        "B"
    }

    fun b_call() {
        "b_call\n"
    }
}

class C : B {
}

class D : A, B {
}

class E : C {}

a = A() // "A"
"\n"
a
"\n"
b = B() // "B"
"\n"
b
"\n"
c = C() // "B"
"\n"
c
"\n"
d = D() // "A"
"\n"
d
"\n"

~d.b_call()
~d.a_call()

~c.a_call()

e = E() // "B"
"\n"

~E.b_call(e)