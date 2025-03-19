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
            ~super()
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