///
/// \brief Moss standard library
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///

__LIB_MS_VERSION = "0.0.2"

@internal_bind("Int")
class __Int {
    @internal
    fun Int(v:String, base=10) {}

    @internal
    fun Int(v=0) {}
}

@internal_bind("Float")
class __Float {
}

@internal_bind("Bool")
class __Bool {
}

@internal_bind("String")
class __String {
}

@internal_bind("List")
class __List {
    @internal
    fun length() {}
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
fun input(prompt="") {}

@internal
fun exit(code=0) {}

@internal_bind("File")
class __File {
    enum FileMode {
        READ        // "r"
        WRITE       // "w"
        CREATE      // "x"
        APPEND      // "a"
        READ_WRITE  // "+"
    }

    fun File(path:String, mode:String="r") {
        this.path = path
        this.mode = mode
    }

    //fun File(path:String, mode:FileMode) {
    //    //this.path = path
    //    //this.mode = this.mode2str(mode)
    //}

    //fun mode2str(mode:FileMode) {
    //    if(mode == FileMode.READ) return "r"
    //    if(mode == FileMode.WRITE) return "w"
    //    if(mode == FileMode.CREATE) return "x"
    //    if(mode == FileMode.APPEND) return "a"
    //    if(mode == FileMode.READ_WRITE) return "+"
    //    assert(false, "missing file mode")
    //}

    //fun str2mode(mode:String) {
    //    if(mode == "r") return FileMode.READ
    //    if(mode == "w") return FileMode.WRITE
    //    if(mode == "x") return FileMode.CREATE
    //    if(mode == "a") return FileMode.APPEND
    //    if(mode == "+") return FileMode.READ_WRITE
    //    assert(false, "unknown file mode")
    //}

    //fun write(content:String)

    //fun read(size:Int=-1)

    //fun readln()

    @internal
    fun readlines() {}

    @internal
    fun open() {}

    //fun close()*/
}

fun open(path:String, mode:String="r") {
    f = File(path, mode)
    ~f.open()
    return f
}

@internal
fun rand_int(min:Int, max:Int) {}

/// Exceptions
@internal_bind("Exception")
class __Exception {

    fun Exception(msg="") {
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

@internal_bind("ValueError")
class __ValueError : Exception {
    fun __String() {
        return "ValueError: " ++ this.msg
    }
}

@internal_bind("StopIteration")
class __StopIteration {
}

@internal_bind("MathError")
class __MathError : Exception {
    fun __String() {
        return "MathError: " ++ this.msg
    }
}

@internal_bind("DivisionByZeroError")
class __DivisionByZeroError : MathError {
    fun __String() {
        return "DivisionByZeroError: " ++ this.msg
    }
}

@internal_bind("OSError")
class __OSError : Exception {
    fun __String() {
        return "OSError: " ++ this.msg
    }
}

@internal_bind("FileNotFoundError")
class __FileNotFoundError : OSError {
    fun __String() {
        return "FileNotFoundError: " ++ this.msg
    }
}