// Constructor calls
space test1 {
    class A {
        fun A() {
            ~print("A")
        }
    }

    class B : A {
        fun B() {
            ~print("B ", end="")
            ~super()
        }
    }

    b = B() // "B A"
}

space test2 {
    class A {
        fun A(x) {
            ~print("A", x)
        }
    }

    class B : A {
        
    }

    class C : B {
        fun C(x) {
            ~super(x)
        }
    }

    c = C(2) // A 2
}

space test3 {
    class A {
        fun A() {
            ~print("A")
        }
    }

    class B : A {
        fun B() {
            ~print("B")
        }
    }

    class C : B, A {
        fun C() {
            ~super()
        }
    }

    c = C() // B 
}

space test3 {
    class A {
        fun A() {
            ~print("A")
        }
    }

    class B : A {
        fun B() {
            ~print("B")
        }
    }

    class C : A {
        fun C() {
            ~print("C")
            ~super()
        }
    }

    class D : B, C {
        fun D() {
            ~print("D ", end = "")
            ~super()
        }
    }

    d = D() // B 
}

space test4 {
    class A {

    }

    class B : A {

    }

    class C : B, A {
        fun C() {
            ~print("C")
            ~super()
        }
    }

    c = C() // C
}

space test5 {
    class C  {
        fun C() {
            ~print("C")
            try
                ~super()
            catch(e:TypeError) {}
        }
    }

    c = C() // C
}

space test6 {
    class A {
        fun A() {
            ~print("A")
        }
    }

    class B {

    }

    class C : B, A {
        fun C() {
            ~super()
        }
    }

    c = C() // A
}

space test7 {
    class A {
        VAL = "VAL A\n"

        fun A() {
            ~print("A")
        }

        fun foo() {
            return "A foo " ++ this.a ++ "\n"
        }
    }

    class B : A {
        VAL = "VAL B\n"
    }

    class C : B, A {
        fun C() {
            this.a = 42
        }

        fun get_val() {
            return super.VAL
        }

        fun incorrect() {
            return super.get_val()
        }

        fun goo() {
            return super.foo()
        }
    }

    c = C() // A
    c.goo()
    c.get_val()
}

try test7.c.incorrect(); catch(e:AttributeError) "caught\n"

class ParentC {
    fun ParentC(a, b) {
        "in parent\n"
        this.a = a
        this.b = b
    }
}

class SubC : ParentC {
    fun SubC() {
        ~super(1, "text\n")
        this.a
        "\n"
        this.b
    }
}

subc = SubC()
subc.a
"\n"
subc.b

try {
    subc.super
} catch (e:AttributeError)
    "caught\n"

class Mod1Cls {
    fun foo() {
        return "hi"
    }
}

class ClassExt : Mod1Cls {
    fun get_sup() {
        return super
    }
}

fun get_mod() {
    return ClassExt()
}

get_mod().get_sup()
"\n"
get_mod().get_sup().foo()
"\n"
