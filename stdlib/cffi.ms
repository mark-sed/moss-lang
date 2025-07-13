d"""
CFFI -- C Foreign Function Interface

Module for calling C function from Moss.
"""

class CFFI {
    fun CFFI(handle:cpp.ffi_cif) {
        this.handle = handle
    }

    //fun define(name:String, return_type:cpp)
}

// TODO: Add also flags (such as NODELETE)
@internal
fun dlopen(path:String) {}