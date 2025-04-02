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
    @internal
    fun Float(v=0.0) {}
}

@internal_bind("Bool")
class __Bool {
    @internal
    fun Bool(v=false) {}
}

@internal_bind("String")
class __String {
    @internal
    fun String(v="") {}

    @internal
    fun length() {}

    fun reverse() = this[-1..-this.length()-1]
}

@internal_bind("Note")
class __Note {
    @internal
    fun Note(value:String="", format:String="txt") {}
}

@internal_bind("NilType")
class __NilType {
    @internal
    fun NilType() { }
}

@internal_bind("List")
class __List {
    @internal
    fun List(... vals) {}

    @internal
    fun length() {}

    @internal
    fun append(... v) {}

    @internal
    fun pop(index:Int=-1) {}
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

    fun File(path:String, mode:String="r") {
        this.path = path
        this.mode = mode
    }

    @internal
    fun write(content) {}

    //fun read(size:Int=-1) {}
    //fun readln() {}
    //fun seek(pos:Int) {}

    @internal
    fun readlines() {}

    @internal
    fun open() {}

    @internal
    fun close() {}
}

fun open(path:String, mode:String="r") {
    f = File(path, mode)
    ~f.open()
    return f
}

@internal
fun rand_int(min:Int, max:Int) {}

@internal
fun rand_float(min:[Float,Int]=0.0, max:[Float,Int]=1.0) {}

@internal
fun sleep(seconds:[Float,Int]) {}

space Math {
    /// π (Ludolph's number)
    pi = 3.141592653589793115997963468544185161590576171875

    /// e (Euler's number)
    e = 2.718281828459045090795598298427648842334747314453125

    /// τ (2*pi)
    tau = 6.28318530717958623199592693708837032318115234375

    /// φ (Golden ratio = (1 + 5^0.5)/2)
    phi = 1.6180339887498949025257388711906969547271728515625

    /// Euler–Mascheroni constant
    euler_gamma = 0.57721566490153286554942724251304753124713897705078125

    @internal
    fun sin(x:[Int,Float]) {}

    @internal
    fun tan(x:[Int,Float]) {}

    @internal
    fun cos(x:[Int,Float]) {}

    fun sqrt(x) {
        //d"""
        //Square root function
        //Is equal to x^0.5
        //@param x Value to be calculated the square root of
        //@return Square root of passed in argument
        //"""
        return x^0.5;
    }
}

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

@internal_bind("KeyError")
class __KeyError : LookupError {
    fun __String() {
        return "KeyError: " ++ this.msg
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