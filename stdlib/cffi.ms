d"""
CFFI -- C Foreign Function Interface

Module for calling C function from Moss.
"""

class FFHandle {
    fun FFHandle(ptr:cpp.cvoid_star, cif:cpp.ffi_cif, name:String, return_type, arg_types, ffi_types:cpp.cvoid_star) {
        this.ptr = ptr
        this.cif = cif
        this.name = name
        this.return_type = return_type
        this.arg_types = arg_types
        this.ffi_types = ffi_types
    }

    @internal
    fun call(... args) {}
}

class CFFI {
    fun CFFI(handle:cpp.cvoid_star) {
        this.handle = handle
    }

    @internal
    fun cfun(name:String, return_type, arg_types) {}

    @internal
    fun close() {}
}

// TODO: Add also flags (such as NODELETE)
@internal
fun dlopen(path:String) {}