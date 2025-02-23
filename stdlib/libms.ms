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

@internal_bind("Range")
class __Range {

    fun Range(start:Int, end:Int, step:Int=nil) {
        this.start = start
        this.end = end
        if (step != nil)
            this.step = step
        else if (start <= end)
            this.step = 1
        else
            this.step = -1
        this.i = start
    }

    fun Range(end:Int) {
        this.start = 0
        this.end = end
        if (this.start < end)
            this.step = 1
        else
            this.step = -1
        this.i = this.start
    }

    fun __iter() {
        this.i = this.start
        return this
    }

    fun __next() {
        if (this.step >= 0 and this.i >= this.end) raise StopIteration()
        if (this.step < 0 and this.i <= this.end) raise StopIteration()
        r = this.i
        this.i = this.i + this.step
        return r
    }
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
@internal_bind("Exception")
class __Exception {

    fun Exception(msg) {
        this.msg = msg
    }

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
        return "AssertionError: " ++ this.msg ++ "\n"
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

@internal_bind("LookupError")
class __LookupError : Exception {
    fun __String() {
        return "LookupError: " ++ this.msg
    }
}

@internal_bind("IndexError")
class __IndexError : LookupError {
    fun __String() {
        return "IndexError: " ++ this.msg
    }
}

@internal_bind("StopIteration")
class __StopIteration {
}
