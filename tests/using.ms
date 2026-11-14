class MyClass {

    fun MyClass(c) {
        this.c = c
    }

    fun __enter() {
        ~print("Entered:", this.c)
    }

    fun __exit() {
        ~print("Exited:", this.c)
    }

    fun __String() = "MyClass:"++this.c++"\n"
}

using (x = MyClass(42)) {
    ~print("In the middle of using:", x.c)
}

x=2
using(x){
    x
}
"\n"

using (f = open(args[0])) {
    f.read()
}
using (f = open(args[0])) f.read()
assert(f.__fstream == nil, "using File exit did not close file or fstream is set") // should be nil

class MyClassEnter {

    fun MyClassEnter(c) {
        this.c = c
    }

    fun __enter() {
        ~print("Entered:", this.c)
    }
}

class MyClassExit {

    fun MyClassExit(c) {
        this.c = c
    }

    fun __exit() {
        ~print("Exited:", this.c)
    }
}

using (v = MyClassEnter(9)) {
    ~print("In the middle of using:", v.c)
}

using (v = MyClassExit(7)) {
    ~print("In the middle of using:", v.c)
}

using(4) {}
using(u, i, o = [1, nil, true]) {}

using(a, b = [MyClass(100), MyClass(101)]) {
    a
    b
}

f = [nil, 1]
using(f[0] = MyClass(800)) {
    "Middle of using subsc 800\n"
}
f ++ "\n"

using(i1, ...i2, i3 = [MyClass(-1), MyClass(-2), MyClass(-3), MyClass(-4), MyClass(-5)]) {
}

using(a1, a2, a3, a4 = [MyClass(nil), 0, "hi", MyClass(true)]) {
    a1
    a2
    a3
    a4
}

using (v = MyClass(-7)) {
    ~print("In the middle of using:", v.c)
    if (v.c < 0)
        raise Exception("Negative c")
}
