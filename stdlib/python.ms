d"""
Module for interoperability with Python3.
"""

@internal_bind("PythonObject")
class __PythonObject {
    d"""
    Moss version of a Python object.
    """

    @internal
    fun PythonObject(ptr:cpp.cvoid_star) {}
}

class PythonException : Exception {
    fun PythonException(msg:String, obj:PythonObject=nil) {
        ~super(msg)
        this.obj = obj
    }

    fun __String() {
        return "PythonException: " ++ this.msg ++ "\n"
    }
}

@internal
fun module(name:String) {}

@!internal_module