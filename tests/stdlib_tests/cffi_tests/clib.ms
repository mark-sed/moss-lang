// This is enclosed in a function to test GC on deleting
// cffi values
fun goo() {
    import cffi

    c_lib = cffi.dlopen(args[0])
    c_lib
    "\n"
    c_lib.handle
    "\n"

    ~c_lib.cfun("greet", cpp.cvoid, [])
    c_lib.greet
    "\n"
    ~c_lib.greet.call()

    ~c_lib.cfun("get_num", cpp.clong, [])
    c_lib.get_num.call_moss()
    "\n"

    fun foo(a, b) {
        ~c_lib.cfun("add", cpp.cdouble, [cpp.cdouble, cpp.cdouble])
        c_lib.add(a, b).to_moss()
        "\n"
    }

    ~foo(4.5, 2.2)
    ~foo(100.0, 200.0)
    ~foo(-5.1, 1.3)

    ~c_lib.close()
}

~goo()

import cffi
c_lib = cffi.dlopen(args[0])

~c_lib.cfun("add", cpp.cdouble, [cpp.cdouble, cpp.cdouble])
c_lib.add.call_moss(0.5, 1.5)
"\n"

~c_lib.cfun("add_dl", cpp.cdouble, [cpp.cdouble, cpp.clong, cpp.clong, cpp.clong])
c_lib.add_dl(0.0, 1, 2, 3).to_moss()
"\n"

~c_lib.cfun("add_dl10", cpp.cdouble, [cpp.cdouble, cpp.clong, cpp.clong, cpp.clong, cpp.clong, cpp.clong, cpp.clong, cpp.clong, cpp.clong, cpp.clong])
c_lib.add_dl10.call(0.0, 1, 2, 3, 4, 5, 6, 7, 8, 9).to_moss()
"\n"

~c_lib.cfun("print_msg", cpp.cvoid, [cpp.cchar_star])
~c_lib.print_msg("Hello from moss to C!")

~c_lib.cfun("get_lib_name", cpp.cchar_star, [])
c_lib.get_lib_name.call_moss()
"\n"

r = c_lib.add.call(0.5, 1.5)
r
"\n"
r.to_moss()
"\n"

c_lib.add.call_moss(0.5, 1.5)
"\n"
c_lib.add(1.5, 3.5).to_moss()
"\n"

try {
    // Cannot extend, its sealed
    class Foo : cpp.cdouble {
        fun Foo() {}
    }
} catch (e:TypeError)
    "caught\n"


v = cpp.cchar_star("Hello\n")
v.to_moss()

~c_lib.close()
"done\n"