// This is enclosed in a function to test GC on deleting
// cffi values
fun goo() {
    import cffi

    c_lib = cffi.dlopen("examples/clib.so")
    c_lib
    "\n"
    c_lib.handle
    "\n"

    ~c_lib.cfun("greet", cpp.cvoid, [])
    c_lib.greet
    "\n"
    ~c_lib.greet.call()

    ~c_lib.cfun("get_num", cpp.clong, [])
    c_lib.get_num.call()
    "\n"

    fun foo(a, b) {
        ~c_lib.cfun("add", cpp.cdouble, [cpp.cdouble, cpp.cdouble])
        c_lib.add.call(a, b)
        "\n"
    }

    ~foo(4.5, 2.2)
    ~foo(100.0, 200.0)
    ~foo(-5.1, 1.3)

    ~c_lib.close()
}

~goo()

import cffi
c_lib = cffi.dlopen("examples/clib.so")

~c_lib.cfun("add", cpp.cdouble, [cpp.cdouble, cpp.cdouble])
c_lib.add.call(0.5, 1.5)
"\n"

// TODO: Fix
//~c_lib.cfun("add_dl", cpp.clong, [])
//c_lib.add_dl.call()
//"\n"

~c_lib.close()
"done\n"