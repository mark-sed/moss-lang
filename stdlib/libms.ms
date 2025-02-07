///
/// \brief Moss standard library
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///

__LIB_MS_VERSION = "0.0.1"

@internal_bind("Int")
class __Int {
    @internal
    fun Int(v:String, base=10) {}

    @internal
    fun Int(v:[Int, Float]=0) {}
}

/*@internal_bind("Float")
class __Float {
    @internal
    fun Float(v:String) {}

    @internal
    fun Float(v:[Int, Float]=0.0) {}
}*/

@internal
fun vardump(value) {}

@internal
fun print(... msgs, end="\n", separator=" ") {}

@internal
fun exit(code=0) {}

/// Exceptions
// TODO: Place into space
@internal_bind("Exception")
class __Exception {
    @internal
    fun Exception(msg:String) {}

    fun __String() {
        return "Exception: "++this.msg
    }
}

@internal_bind("NameError")
class __NameError : Exception {
    fun __String() {
        return "NameError: " ++ this.msg
    }
}

@internal_bind("AttributeError")
class __AttributeError : NameError {
    // TODO: Perhaps set `obj` and `name` attrs
    // for easy extraction
    fun __String() {
        return "AttributeError: " ++ this.msg
    }
}

@internal_bind("ModuleNotFoundError")
class __ModuleNotFoundError : NameError {
    fun __String() {
        return "ModuleNotFoundError: " ++ this.msg
    }
}

@internal_bind("TypeError")
class __TypeError : Exception {
    fun __String() {
        return "TypeError: " ++ this.msg
    }
}

@internal_bind("AssertionError")
class __AssertionError : Exception {
    fun __String() {
        return "AssertionError: " ++ this.msg
    }
}

@internal_bind("NotImplementedError")
class __NotImplementedError : Exception {
    fun __String() {
        return "NotImplementedError: " ++ this.msg
    }
}

@internal_bind("ParserError")
class __ParserError : Exception {
    fun __String() {
        return "ParserError: " ++ this.msg
    }
}

@internal_bind("SyntaxError")
class __SyntaxError : ParserError {
    fun __String() {
        return "SyntaxError: " ++ this.msg
    }
}
