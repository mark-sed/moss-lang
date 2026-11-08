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

using (v = MyClass(-7)) {
    ~print("In the middle of using:", v.c)
    if (v.c < 0)
        raise Exception("Negative c")
}