d"""
CFFI -- C Foreign Function Interface

Module for calling C function from Moss.
"""

class FFHandle {
    fun FFHandle(ptr:cpp.cvoid_star, cif:cpp.ffi_cif) {
        this.ptr = ptr;
        this.cif = cif
    }

    //@internal
    //fun call() {}
}

class CFFI {
    fun CFFI(handle:cpp.cvoid_star) {
        this.handle = handle
    }

    @internal
    fun define(name:String, return_type, ... arg_types) {}

    @internal
    fun close() {}
}

// TODO: Add also flags (such as NODELETE)
@internal
fun dlopen(path:String) {}