d"""
Moss standard library
author: Marek Sedlacek
copyright: Copyright 2024 Marek Sedlacek. All rights reserved.
           See accompanied LICENSE file.
"""

__LIB_MS_VERSION = "0.0.6"

@internal_bind("Int")
class __Int {
    @internal
    fun Int(v:String, base=10) {
        d"Converts String value `v` of number base `base` into na Int."
    }

    @internal
    fun Int(v=0) {
        d"""
        Converts value `v` into Int.
        If `v` is not Int, Float or Object which contains __Int function then TypeError is raised.
        When converting Float, then the decimal part is cut of, not rounded.
        """
    }
}

@internal_bind("Float")
class __Float {
    @internal
    fun Float(v=0.0) {
        d"""
        Converts value `v` into Float value.
        If `v` is not Float, Int, String or Object which contains __Float function then TypeError is raised.
        """
    }
}

@internal_bind("Bool")
class __Bool {
    @internal
    fun Bool(v=false) {
        d"""
        Converts value `v` into Bool value (true or false).
        TypeError is raised if `v` is not one of these types or Object which contains __Bool function:
          - Bool.
          - Int -- 0 is false, other are true.
          - Float -- 0.0 is false, other are true.
          - NilType (nil) -- always false.
          - String -- empty string is false, otherwise it's true.
          - List -- empty list is false, otherwise it's true.
          - Dict -- empty dict is false, otherwise it's true.
        """
    }
}

@internal_bind("String")
class __String {
    @internal
    fun String(v="") {
        d"""
        Converts value `v` into String.
        This functions behaves the same as concatinating with empty String and will always succeed for any type, as
        __String method is called or implicit to string converter is used.
        """
    }

    @internal
    fun length() {
        d"Returns amount of characters in this string."
    }

    fun empty() {
        d"Returns true if String is empty, false otherwise."
        return this.length() == 0
    }

    @internal
    fun strip() {
        d"Removes whitespace (' ', '\f', '\n', '\r', '\t', '\v') characters from start and end of String."
    }

    fun reverse() = this[-1..-this.length()-1]

    @internal
    fun capitalize() {
        d"Makes the first letter uppercase and the rest of the String lowercase."
    }

    @internal
    fun upper() {
        d"Converts all the string letters into uppercase."
    }

    @internal
    fun lower() {
        d"Converts all the string letters into lowercase."
    }

    @internal
    fun replace(target:String, value:String, count:Int=-1) {
        d"""
        Replaces all the substrings equal to `target` with `value`.
        When `count` is specified, then this replacing will happen that many times.
        """
    }

    @internal
    fun multi_replace(mapping:List) {
        d"""
        Replaces all substrings in `mapping` element at index 0 with string at index 1.
        @param mapping List of mappings of value to replace with value to replace it with. So it should be a list of
                       pairs (lists of size 2). 
        Example:
        ```
        angles = "a: 45°, b: 75°"
        angles.multi_replace([["a", "alfa"], ["b", "beta"]])
        ```
        """
    }

    fun join(iterable) {
        d"""
        Converts values in `iterable` into String and concatenates these values together using this String as the
        connector.
        Example:
        ```
        ", ".join([1,2,3,4])
        ```
        """
        // TODO: Internal candidate
        r = ""
        first = true
        for (i: iterable) {
            if (first) { 
                r ++= i
                first = false
            }
            else r ++= this ++ i
        }
        return r
    }

    @internal
    fun isspace() {
        d"""
        Returns true if all the characters in this string are whitespace (' ', '\f', '\n', '\r', '\t', '\v').
        """
    }

    @internal
    fun split(sep:[String,NilType]=nil, maxsplit:Int=-1) {
        d"""
        Returns a list of strings, using `sep` as the delimiter string.
        If `sep` is nil, then whitespace is used as the separator.
        If `sep` in empty, then string is split at every character.
        If `maxsplit` is non-negative number then split is done this many times.
        """
    }

    @internal
    fun __iter() {}
    
    @internal
    fun __next() {}
}

@internal_bind("Note")
class __Note : String {
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
    fun List() {}
    @internal
    fun List(iterable) {}

    @internal
    fun length() {}

    fun empty() = this.length() == 0

    @internal
    fun append(... v) {}

    fun copy() = ::copy(this)

    @internal
    fun pop(index:Int=-1) {}

    fun reverse() = this[-1..-this.length()-1]

    @internal
    fun __iter() {}

    @internal
    fun __next() {}

    @internal
    fun clear() {}
}

@internal_bind("Dict")
class __Dict {
    @internal
    fun Dict() {}
    @internal
    fun Dict(iterable) {}

    @internal
    fun clear() {}

    fun copy() = ::copy(this)

    @internal
    fun length() {}

    fun empty() = this.length() == 0

    @internal
    fun get(key, def_val=nil) {}

    @internal
    fun pop(key) {}
    @internal
    fun pop(key, def_val) {}

    fun remove(key) {
        ~this.pop(key)
    }
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

class Enumerate {
    fun Enumerate(iterable, start:Int=0) {
        this.iterable = iterable
        this.start = start
    }

    fun __iter() {
        this.i = this.start
        this.iter = this.iterable.__iter()
        return this
    }

    fun __next() {
        r = [this.i, this.iter.__next()]
        this.i += 1
        return r
    }
}

class Complex {
    fun Complex(real:[Int,Float]=0, imag:[Int,Float]=0) {
        this.real = real
        this.imag = imag
    }

    // TODO:
    // @internal
    //fun Complex(value:String) {}

    fun (==)(other:Complex) {
        return this.real == other.real and this.imag == other.imag
    }
    fun (==)(other:[Float,Int]) {
        return this.real == other and this.imag == 0
    }

    fun (!=)(other:Complex) {
        return this.real != other.real or this.imag != other.imag
    }
    fun (!=)(other:[Float,Int]) {
        return this.real != other or this.imag != 0
    }

    fun (+)(other:Complex) {
        return ::Complex(this.real + other.real, this.imag + other.imag)
    }
    fun (+)(other:[Float,Int]) {
        return ::Complex(this.real + other, this.imag)
    }

    fun (-)(other:Complex) {
        return ::Complex(this.real - other.real, this.imag - other.imag)
    }
    fun (-)(other:[Float,Int]) {
        return ::Complex(this.real - other, this.imag)
    }
    fun (-)() {
        return ::Complex(-this.real, -this.imag)
    }

    fun (*)(other:Complex) {
        r = this.real * other.real - this.imag * other.imag
        i = this.real * other.imag + this.imag * other.real
        return ::Complex(r, i)
    }
    fun (*)(other:[Float,Int]) {
        return ::Complex(this.real * other, this.imag * other)
    }

    fun (/)(other:Complex) {
        denom = other.real ^ 2.0 + other.imag ^ 2.0
        if (denom == 0)
            raise DivisionByZeroError("Complex division by zero")
        r = (this.real * other.real + this.imag * other.imag) / denom
        i = (this.imag * other.real - this.real * other.imag) / denom
        return ::Complex(r, i)
    }
    fun (/)(other:[Float,Int]) {
        denom = other ^ 2.0
        if (denom == 0)
            raise DivisionByZeroError("Complex division by zero")
        r = (this.real * other) / denom
        i = (this.imag * other) / denom
        return ::Complex(r, i)
    }

    fun __String() {
        return this.real ++ (this.imag >= 0 ? "+" : "") ++ this.imag ++ "i"
    }
}

@internal_bind("Type")
class __Type {
    //@internal
    //fun Type() { }
}

@internal
fun vardump(value) {}

@internal
fun print(... msgs, end="\n", separator=" ") {}

@internal
fun input(prompt="") {}

fun exit(code=0) {
    raise SystemExit(code)
}

@internal
fun round(n:[Int,Float], ndigits:[Int,NilType]=nil) {}

@internal
fun copy(obj) {}

@internal
fun hash(obj) {}

@internal
fun type(obj) {}

@internal
fun id(obj) {}

@internal
fun isinstance(obj, types) {}

@internal
fun issubclass(cls:Type, types) {}

@internal
fun attrs() {}
@internal
fun attrs(obj) {}

@internal
fun hasattr(obj, name:String) {}

@internal
fun getattr(obj, name:String) {}

@internal
fun setattr(obj, name:String, value) {}

@internal
fun delattr(obj, name:String) {}

@internal
fun callable(obj) {}

@internal
fun lshift(a:Int, count:Int) {
    d"""
    Left shift.
    @param a Int value.
    @param count Amount to shift by.
    @return First argument shifted left by the amount in the second argument.
    """
}

@internal
fun rshift(a:Int, count:Int) {
    d"""
    Right shift.
    @param a Int value.
    @param count Amount to shift by.
    @return First argument shifted right by the amount in the second argument.
    """
}

fun all(iterable, test:Function) {
    for (i : iterable) {
        if (not test(i))
            return false
    }
    return true
}

fun all(iterable) {
    for (i: iterable) {
        if (not i)
            return false
    }
    return true
}

fun any(iterable, test:Function) {
    for (i : iterable) {
        if (test(i))
            return true
    }
    return false
}

fun any(iterable) {
    for (i: iterable) {
        if (i)
            return true
    }
    return false
}

fun length(iterable) {
    l = 0
    for (_: iterable)
        l += 1
    return l
}

enum ValType {
    MISSING,
    NONE
}

// TODO: Can be optimized
fun max(iterable, key=nil, def_val=ValType.MISSING) {
    curr_max = ValType.NONE
    for (i: iterable) {
        if (curr_max == ValType.NONE || (key != nil && key(i) > key(curr_max)) || i > curr_max) {
            curr_max = i
        }
    }
    if (curr_max == ValType.NONE && def_val != ValType.MISSING)
        return def_val
    else if (curr_max == ValType.NONE)
        raise ValueError("max() iterable is empty\n")
    return curr_max
}

fun min(iterable, key=nil, def_val=ValType.MISSING) {
    curr_max = ValType.NONE
    for (i: iterable) {
        if (curr_max == ValType.NONE || (key != nil && key(i) < key(curr_max)) || i < curr_max) {
            curr_max = i
        }
    }
    if (curr_max == ValType.NONE && def_val != ValType.MISSING)
        return def_val
    else if (curr_max == ValType.NONE)
        raise ValueError("max() iterable is empty\n")
    return curr_max
}

@internal
fun chr(i:Int) {}

@internal
fun ord(c:String) {}

@internal
fun hex(number:Int) {}

@internal
fun oct(number:Int) {}

@internal
fun bin(number:Int) {}

fun info(v, indent:String="") {
    d"""
    Returns formatted doc string or list of pairs of [function, doc strings] for overloaded function.
    For multi-line doc string indentation is removed based on indentation of the first line.
    """
    fun get_doc(a) {
        docs = ""
        try {
            // This is needed if __doc is changed by hand and also for libms docs as those don't have correct signature.
            docs = String(a.__doc)
        } catch(e:AttributeError) {
            return ""
        }
        // Extract indentation
        splitted_all = docs.split("\n")
        if (splitted_all.length() == 1)
            splitted = splitted_all
        else {
            // Remove new lines from start and end of the doc string
            start = 0
            end = splitted_all.length()-1
            for (start : 0..splitted_all.length()) {
                if (not splitted_all[start].empty())
                    break
            }
            // Also remove ws from the end lines, no need for that
            for (i : splitted_all.length()-1..start) {
                if (not splitted_all[i].empty() && not splitted_all[i].isspace()) {
                    end = i
                    break
                }
            }
            splitted = splitted_all[start..end+1]
        }
        indent_found = ""
        for (c: splitted[0]) {
            if (not c.isspace())
                break
            indent_found ++= c
        }
        formatted = [indent ++ l.replace(indent_found, "", 1) : l = splitted]
        dstring = "\n".join(formatted)
        if (not dstring.empty() && dstring[-1] != "\n")
            return dstring++"\n"
        return dstring
    }

    if (type(v) == FunctionList) {
        docs = []
        for (f: v) {
            ~docs.append([f, get_doc(f)])
        }
        return docs;
    } else {
        return get_doc(v)
    }
}

fun help(a) {
    import inspect
    // TODO: list attributes and their info
    txt = ""
    if (type(a) == Function) {
        txt = inspect.signature(a) ++ "\n"
        txt ++= info(a, "    ")
    } else if (type(a) == FunctionList) {
        for (f: a) {
            txt ++= inspect.signature(f) ++ "\n"
            txt ++= info(f, "    ")
            txt ++= "\n"
        }
    } else if (type(a) == Type) {
        // TODO: Add parents
        txt ++= "class " ++ inspect.name(a) ++ "\n"
        txt ++= info(a, "    ")
    } else if (type(a) == Enum) {
        txt ++= "enum " ++ inspect.name(a) ++ " {"
        for (e: inspect.enum_values(a)) {
            txt ++= "\n    " ++ e
        }
        if (not inspect.enum_values(a).empty())
            txt ++= "\n"
        txt ++= "}\n"
    } else if (type(a) == Space) {
        txt ++= "space " ++ inspect.name(a) ++ "\n"
        txt ++= info(a, "    ")
    } else if (type(a) == Module) {
        txt ++= "Module " ++ inspect.name(a) ++ "\n"
        txt ++= info(a, "    ")
    } else {
        txt ++= info(a, "    ")
    }
    return txt
}

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

fun with_open(path:String, func, mode:String="r") {
    file = open(path, mode)
    try {
        return func(file)
    } catch(v) {
        raise v
    } finally {
        ~file.close()
    }
}

@internal
fun rand_int(min:Int, max:Int) {}

@internal
fun rand_float(min:[Float,Int]=0.0, max:[Float,Int]=1.0) {}

@internal
fun sleep(seconds:[Float,Int]) {}

space Math {
    d"""
    Space for Mathematical resources and functions.
    """

    /// π (Ludolph's number).
    pi = 3.141592653589793115997963468544185161590576171875

    /// e (Euler's number).
    e = 2.718281828459045090795598298427648842334747314453125

    /// τ (2*pi).
    tau = 6.28318530717958623199592693708837032318115234375

    /// φ (Golden ratio = (1 + 5^0.5)/2).
    phi = 1.6180339887498949025257388711906969547271728515625

    /// Euler–Mascheroni constant.
    euler_gamma = 0.57721566490153286554942724251304753124713897705078125

    @internal
    fun sin(x:[Int,Float]) {
        d"""
        Sine.
        @param x Numeric value.
        @return Sine of x in radians.
        """
    }

    @internal
    fun tan(x:[Int,Float]) {
        d"""
        Tangent.
        @param x Numeric value.
        @return Tangent of x in radians.
        """
    }

    @internal
    fun cos(x:[Int,Float]) {
        d"""
        Cosine.
        @param x Numeric value.
        @return Cosine of x in radians.
        """
    }

    fun sqrt(x) {
        d"""
        Square root function.
        Is equal to x^0.5.
        @param x Value to be calculated the square root of.
        @return Square root of passed in argument.
        """
        return x^0.5;
    }

    @internal
    fun abs(n:[Int,Float]) {}

    fun sum(nums, start:[Int,Float]=0) {
        for (i : nums) {
            start += i
        }
        return start
    }

    @internal
    fun log(x:[Int,Float], base:[Int,Float]) {}

    @internal
    fun divmod(x:[Int,Float], y:[Int,Float]) {}
}

space Converters {
d"""
Space for converters for moss known formats.
These converters should provide basic file generation.

These can be easily overridden by custom converters in user scope.
"""

@converter("txt", "md")
@converter("txt", "markdown")
fun txt2textual(n) {
    if (type(n) != String)
        return n
    markdown_escapes = [
        ["\\", "\\\\"],
        ["`", "\\`"],
        ["*", "\\*"],
        ["_", "\\_"],
        ["{", "\\{"],
        ["}", "\\}"],
        ["[", "\\["],
        ["]", "\\]"],
        ["(", "\\("],
        [")", "\\)"],
        ["#", "\\#"],
        ["+", "\\+"],
        ["-", "\\-"],
        [".", "\\."],
        ["!", "\\!"],
        [">", "\\>"],
        ["|", "\\|"],
        ["~", "\\~"]
    ]
    return Note(n.multi_replace(markdown_escapes), "md")
}

@converter("txt", "html")
@converter("txt", "html5")
@converter("txt", "html4")
fun txt2html(txt) {
    if (type(txt) != String)
        return txt
    // Not using &apos; to be compatible with HTML4
    replace_mappings = [
        ["&", "&amp;"],
        ["<", "&lt;"],
        [">", "&gt;"],
        ["\"", "&quot;"],
        ["'", "&#39"],
        ["\n", "<br>"]
    ]
    return Note("<p>" ++ txt.multi_replace(replace_mappings) ++ "</p>\n", "html")
}

@converter("md", "html")
@converter("md", "html5")
@converter("md", "html4")
fun md2html(txt) {
    import md_parser
    parsed = md_parser.parse_md(txt)
    converted = [l.to_html() : l = parsed]
    return "\n".join([String(x) : x = converted])
}

}

space Generators {
d"""
Space for generators for moss known formats.
These generators should provide basic file generation.

These can be easily overridden by custom generators in user scope.
"""

space HTML {
    STYLE_PATH="" //< This value will be set based on the platform 
    TITLE=""
    FOOTER=nil
}

@generator("html")
fun html_gen(txts) {
    import sys.version
    import time.strftime
    
    Note(f"""\
<!DOCTYPE html>
<html>
<head>{ HTML.TITLE.length() != 0 ? "<title>"++HTML.TITLE++"</title>" : "" }
    <link rel="stylesheet" href="{HTML.STYLE_PATH}">
</head>
<body>
""", "html")
    for (t: txts) {
        t
    }
    if (HTML.FOOTER == nil)
        footer = f"""Page generated on {strftime("%b %-d %Y %H:%M:%S")} by <a href="https://github.com/mark-sed/moss-lang">Moss</a> {version}."""
    else
        footer = HTML.FOOTER
    Note(f"""\
</body>
{ footer.length() != 0 ? "<footer>"++footer++"</footer>" : "" }
</html>
""", "html")
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

@internal_bind("EOFError")
class __EOFError : Exception {
    fun __String() {
        return "EOFError: " ++ this.msg
    }
}

@internal_bind("SystemExit")
class __SystemExit : Exception {
    fun SystemExit(code=0) {
        this.msg = this.code = code
    }

    fun __String() {
        return String(this.code)
    }
}