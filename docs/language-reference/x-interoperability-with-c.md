# Interoperability with C

_clib.c_:
```c
#include <stdio.h>

void greet() {
    puts("Hello from C!\n");
}
```

Build with:
```sh
gcc -fPIC -shared -o clib.so clib.c 
```

_test.ms_:
```cpp
import cffi

c_lib = cffi.dlopen("./clib.so")
c_lib.cdef("greet", cffi.types.cvoid, []);
c_lib.greet()
```