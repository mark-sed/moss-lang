import cffi

c_lib = cffi.dlopen(args[0])
c_lib
"\n"
c_lib.handle
"\n"

~c_lib.cfun("greet", cpp.cvoid)
c_lib.greet
"\n"
~c_lib.greet.call()

~c_lib.cfun("get_num", cpp.clong)
c_lib.get_num.call()
"\n"

~c_lib.close()