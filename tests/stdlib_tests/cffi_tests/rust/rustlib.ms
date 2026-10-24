import cffi

rust_lib = cffi.dlopen(args[0])
~rust_lib.cfun("add", cpp.cint32_t, [cpp.cint32_t, cpp.cint32_t])
rust_lib.add(8, 34).to_moss()
"\n"

~rust_lib.cfun("print_and_length", cpp.cuint64_t, [cpp.cchar_star])
r = rust_lib.print_and_length("Hello from rust!")
r.to_moss()
"\n"