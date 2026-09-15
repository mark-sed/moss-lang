d"""
CFFI -- C Foreign Function Interface

Module for calling foreign function that use C ABI from Moss.
"""

class CFFIError : Exception {
    d"""
    Exception for CFFI specific errors.
    """

    fun CFFIError(msg:String) {
        ~super(msg)
    }

    fun __String() {
        return "CFFIError: "++this.msg
    }
}

class ForeignFunction {
    d"""
    Class for holding foreign function handle.
    """

    fun ForeignFunction(ptr:cpp.cvoid_star, cif:cpp.ffi_cif, name:String,
                        return_type, arg_types, ffi_types:cpp.cvoid_star) {
        this.ptr = ptr
        this.cif = cif
        this.name = name
        this.return_type = return_type
        this.arg_types = arg_types
        this.ffi_types = ffi_types
    }

    @internal
    fun call(... args) {
        d"""
        Calls foreign function using C ABI and passing in passed in arguments
        converted from Moss type (from cpp space or built-in ones) into C type.
        Returned value is mapped to cpp space type if avaliable otherwise
        exception is raised.
        To get built-in type use `to_moss` method or `call_moss`.
        """
        // Arguments of this function will be modified to match the actual
        // function arguments once object is created.
    }

    @internal
    fun call_moss(... args) {
        d"""
        Calls foreign function and converts returned cpp value into built-in
        value (e.g.: cpp.cint16_t -> Int).
        """
        // Arguments of this function will be modified to match the actual
        // function arguments once object is created.
    }

    @internal
    fun (())(... args) {
        d"""
        Calls foreign function using C ABI and passing in passed in arguments
        converted from Moss type (from cpp space or built-in ones) into C type.
        Returned value is mapped to cpp space type if avaliable otherwise
        exception is raised.
        To get built-in type use `to_moss` method or `call_moss`.
        """
        // Arguments of this function will be modified to match the actual
        // function arguments once object is created.
    }
}

class CFFI {
    d"""
    Class representing C Foreign Function Interface.
    This is aquired by `dlopen`.
    """

    fun CFFI(handle:cpp.cvoid_star) {
        this.handle = handle
    }

    @internal
    fun cfun(name:String, return_type, arg_types:List) {
        d"""
        Declares a foreign C ABI function `return_type name(<args of arg_types>);`.
        If function `name` is not found then NameError is raised.
        Arguments `return_type` and value in `arg_types` list has to be types
        with known conversion to C type (some built-in types or cpp space
        values). If there is no known conversion then TypeError is raised.
        """
    }

    @internal
    fun close() {
        d"""
        Closes handle for further operations.
        This should be called after working with CFFI.
        """
    }
}

// TODO: Add also flags (such as NODELETE)
@internal
fun dlopen(path:String) {
    d"Loads a dynamic shared object (shared library)."
}