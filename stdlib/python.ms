d"""
Module for interoperability with Python3.

The main type here is PythonObject, which represents Python 3 objects including
modules, values and functions. This object can be created initially with
function `module`, which imports a Python module and then by calling `def` or
`get` on this object and accessing it's attributes.
"""

// This annotation has to be before any calls to python runtime so that
// it can initialize it.
@!internal_module

@internal_bind("PythonObject")
class __PythonObject {
    d"Moss version of a Python object."

    @internal
    fun PythonObject(ptr:cpp.cvoid_star, populate:Bool=false) {}
    @internal
    fun PythonObject(v, populate:Bool=false) {
        d"""
        Converts Moss value `v` into Python version of this object.
        If no such conversion exists, then MossToPythonConversionError is
        raised.
        When populate is true, then `populate` method is called.
        """
    }

    @internal
    fun get(name:String) {
        d"""
        Accesses an attribute inside of this object.
        This works just as dot (`.`) operator in Python.
        """
    }

    fun def(name:String) {
        d"""
        Sets attribute `name` of this object to extracted one using `get`.
        """
        a = this.get(name)
        ~setattr(this, name, a)
    }

    @internal
    fun populate() {
        d"""
        Populates this object with all the attributes inside of Python version
        of this value.
        """
    }

    @internal
    fun to_moss() {
        d"""
        Converts PythonObject into a Moss value.
        If no such conversion exists, then PythonToMossConversionError is
        raised.
        """
    }

    @internal
    fun call(... args) {
        d"""
        Function call to this object with passed in arguments.
        Arguments can be Moss values and those will be converted or
        PythonObject values.
        """
    }

    fun call_moss(... args) {
        d"""
        Function call to this object and then conversion of returned value into
        a Moss value.
        """
        rval = this.call(<<args)
        return rval.to_moss()
    }

    fun (())(... args) {
        return this.call_moss(<<args)
    }
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
fun module(name:String, populate:Bool=false) {
    d"""
    Imports and returns python module by name `name`.
    If such module does not exists or there is an error, then PythonException is
    raised.
    """
}

fun to_py(v, populate:Bool=false) {
    d"""
    Converts Moss value to Python value.
    This is an equivalent of calling PythonObject constructor.
    """
    return PythonObject(v, populate)
}

// Python stdlib (implicitly imported values)
stdlib = module("builtins")
~stdlib.populate()