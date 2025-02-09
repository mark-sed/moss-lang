class Foo {
    fun Foo(id) {
        "constructed\n"
        this.id = id
    }

    fun get_id() {
        return this.id
    }
}

foo = Foo(56)
foo.id
"\n"
foo.get_id()
"\n"
Foo.get_id(foo)
"\n"
foo
"\n"

class A {
    fun A(b) {
        this.b = b
    }
    fun get_b(){
        return this.b
    }
}

class B {
    fun B(id) {
        this.id = id
    }
    fun get_id() {
        return this.id
    }
}

b = B(91)
a = A(b)

a.b
"\n"

a.b.id++"\n"
a.get_b().id
"\n"
//a.b.id = 1 // TODO when assignment is done
//a.b.get_id()
//"\n"

b2 = B(65)
B.get_id(b)
" != "
B.get_id(b2)
"\n"

class C {
}
c = C()
c.id = -1
c.id
" == "
// We can use ducktyping
B.get_id(c)
"\n"

// Check that store attr is correct
class FooAtt {
    fun FooAtt() {
        this.a = 4
        this.a
    }

    fun att() {
        this.b = this.a + 1
        this.b
        "\n"
        this.a
        "\n"
    }
}

f = FooAtt()
"\n"
FooAtt.att(f)
"\n"
f.att()
"\n"