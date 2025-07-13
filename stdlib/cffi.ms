d"""
CFFI -- C Foreign Function Interface

Module for calling C function from Moss.
"""

class CFFI {
    fun CFFI(handle:cpp.void_star) {
        this.handle = handle
    }
}

// TODO: Add also flags (such as NODELETE)
@internal
fun dlopen(path:String) {}