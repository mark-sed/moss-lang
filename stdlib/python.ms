d"""
Module for interoperability with Python3.
"""

@internal_bind("PythonObject")
class __PythonObject {
    d"Moss version of a Python object."

    @internal
    fun PythonObject(ptr:cpp.cvoid_star) {}

    @internal
    fun get(name:String) {
        d"""
        Accesses an attribute inside of this object.
        This works just as dot (`.`) operator in Python.
        """
    }

    @internal
    fun to_moss() {}

    @internal
    fun call(... args) {}

    //fun (())(... args) {
    //    return this.call(<<args)
    //}
}

class PythonException : Exception {
    d"Moss exception that represents a Python exception."

    fun PythonException(msg:String, obj:PythonObject=nil) {
        ~super(msg)
        this.obj = obj
    }

    fun __String() {
        return "PythonException: " ++ this.msg ++ "\n"
    }
}

class PythonToMossConversionError : PythonException {
    fun PythonToMossConversionError(msg:String) {
        this.msg = msg
        this.obj = nil
    }

    fun __String() {
        return "PythonToMossConversionError: " ++ this.msg ++ "\n"
    }
}

class MossToPythonConversionError : PythonException {
    fun MossToPythonConversionError(msg:String) {
        // TODO: Use super one fixed
        this.msg = msg
        this.obj = nil
    }

    fun __String() {
        return "MossToPythonConversionError: " ++ this.msg ++ "\n"
    }
}

@internal
fun module(name:String) {
    d"""
    Imports and returns python module by name `name`.
    If such module does not exists or there is an error, then PythonException is
    raised.
    """
}

@!internal_module