d"""
Moss standard library
author: Marek Sedlacek
copyright: Copyright 2024 Marek Sedlacek. All rights reserved.
           See accompanied LICENSE file.
"""

__LIB_MS_VERSION = "0.1.0"

@internal_bind("Int")
class __Int {
    d"""
    Integer (whole number) value.
    """

    @internal
    fun Int(v:String, base=10) {
        d"Converts String value `v` of number base `base` into na Int."
    }

    @internal
    fun Int(v=0) {
        d"""
        Converts value `v` into Int.
        If `v` is not Int, Float or Object which contains __Int function then
        TypeError is raised.
        When converting Float, then the decimal part is cut of, not rounded.
        """
    }
}

@internal_bind("Float")
class __Float {
    d"""
    Floating point value.
    Internally this is double precision floating point with accordance with
    IEEE 754.
    """

    @internal
    fun Float(v=0.0) {
        d"""
        Converts value `v` into Float value.
        If `v` is not Float, Int, String or Object which contains __Float
        function then TypeError is raised.
        """
    }
}

@internal_bind("Bool")
class __Bool {
    d"""
    Boolean value with 2 possible singleton values - true or false. 
    """

    @internal
    fun Bool(v=false) {
        d"""
        Converts value `v` into Bool value (true or false).
        TypeError is raised if `v` is not one of these types or Object which
        contains __Bool function:
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
    d"""
    String value which can contain UNICODE values.
    """

    @internal
    fun String(v="") {
        d"""
        Converts value `v` into String.
        This functions behaves the same as concatinating with empty String and
        will always succeed for any type, as
        __String method is called or implicit to string converter is used.
        """
    }

    @internal
    fun capitalize() {
        d"""
        Makes the first letter uppercase and the rest of the String lowercase.
        This function works with UNICODE.
        """
    }

    @internal
    fun count(sub:String) {
        d"""
        Returns number of non-overlapping occurrences of `sub` in this string.
        """
    }

    fun empty() {
        d"Returns true if String is empty, false otherwise."
        return this.length() == 0
    }

    @internal
    fun index(value:String) {
        d"""
        Returns index of `value` in this String.
        """
    }

    fun index(value:String, start:Int, end:Int) {
        d"""
        Returns index of `value` in this String.
        If `start` is specified then finding starts from this index.
        If `end` is specified then finding ends at this index.
        """
        inx = this[start..end].index(value)
        if (inx <= -1)
            return inx
        return inx + start
    }

    @internal
    fun isalpha() {
        d"""
        Returns true if all the characters in the String are alphabetic.
        This function works with UNICODE in current locale.
        """
    }

    @internal
    fun isalnum() {
        d"""Returns true if all the characters in the String are alphabetic
        or numeric.
        This function works with UNICODE in current locale.
        """
    }

    @internal
    fun isdigit() {
        d"Returns true if all the characters in the String are digits."
    }

    @internal
    fun islower() {
        d"""
        Returns true if all the characters in the String are lowercase.
        This function works with UNICODE in current locale.
        """
    }

    @internal
    fun isprintable() {
        d"Returns true if all the characters in the String are printable."
    }

    @internal
    fun isspace() {
        d"""
        Returns true if all the characters in this string are whitespace 
        (' ', '\f', '\n', '\r', '\t', '\v'). This function works with UNICODE
        in current locale.
        """
    }

    @internal
    fun isupper() {
        d"""
        Returns true if all the characters in the String are uppercase.
        This function works with UNICODE in current locale.
        """
    }

    fun join(iterable) {
        d"""
        Converts values in `iterable` into String and concatenates these values
        together using this String as the connector.
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
    fun length() {
        d"Returns amount of characters in this string."
    }

    @internal
    fun lower() {
        d"""
        Converts all the string letters into lowercase.
        This function works with UNICODE.
        """
    }

    @internal
    fun multi_replace(mapping:List) {
        d"""
        Replaces all substrings in `mapping` element at index 0 with string at
        index 1.
        @param mapping List of mappings of value to replace with value to 
                replace it with. So it should be a list of pairs
                (lists of size 2). 
        Example:
        ```
        angles = "a: 45°, b: 75°"
        angles.multi_replace([["a", "alfa"], ["b", "beta"]])
        ```
        """
    }

    @internal
    fun replace(target:String, value:String, count:Int=-1) {
        d"""
        Replaces all the substrings equal to `target` with `value`.
        When `count` is specified, then this replacing will happen that many
        times.
        """
    }

    fun reverse() = this[-1..-this.length()-1]

    @internal
    fun rindex(value:String) {
        d"""
        Returns last index of `value` in this String.
        """
    }

    fun rindex(value:String, start:Int, end:Int) {
        d"""
        Returns last index of `value` in this String.
        If `start` is specified then finding starts from this index.
        If `end` is specified then finding ends at this index.
        """
        inx = this[start..end].rindex(value)
        if (inx <= -1)
            return inx
        return inx + start
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
    fun strip() {
        d"""
        Removes whitespace (' ', '\f', '\n', '\r', '\t', '\v') characters from
        start and end of String.
        """
    }

    @internal
    fun swapcase() {
        d"""
        Returns version of this String where lowercase letters are changed to
        uppercase and vice versa.
        This function works with UNICODE.
        """
    }

    @internal
    fun upper() {
        d"""
        Converts all the string letters into uppercase.
        This function works with UNICODE.
        """
    }

    @internal
    fun __iter() {}
    
    @internal
    fun __next() {}
}

@internal_bind("Note")
class __Note : String {
    d"""
    Value for outputting moss notes.
    """

    @internal
    fun Note(value:String="", format:String="txt") {
        d"""
        Creates note with format `format` and contents in `value`.
        The format controls how `value` might be parsed and processed.
        """
    }
}

@internal_bind("NilType")
class __NilType {
    d"""
    Class for singleton value nil.
    """

    @internal
    fun NilType() {
        d"Returns nil."
    }
}

@internal_bind("List")
class __List {
    d"""
    A mutable, dynamically sized, ordered sequence type that can hold
    heterogeneous elements.
    """

    @internal
    fun List() {
        d"Returns empty list."
    }
    @internal
    fun List(iterable) {
        d"Returns list with values in iterable."
    }

    @internal
    fun append(... v) {
        d"Appends all values in `v` to this list."
    }

    @internal
    fun clear() {
        d"""
        Removes all values from this list.
        """
    }

    fun copy() = ::copy(this)

    @internal
    fun count(val) {
        d"""
        Returns how many times does `val` appear in this list (using `==`
        operator).
        """
    }

    @internal
    fun delete(index:[Int,Range]) {
        d"Removes value at index `index` or all values in range."
    }

    fun empty() {
        d"Returns true if list has no values, false otherwise."
        return this.length() == 0
    }

    fun index(value) {
        d"""
        Returns index of `value` in this List.
        """
        i = 0
        for (e: this) {
            if (e == value)
                return i
            i += 1
        }
        raise ValueError(f"Value '{value}' is not in list.\n")
    }

    fun index(value, start:Int, end:Int) {
        d"""
        Returns index of `value` in this List.
        If `start` is specified then finding starts from this index.
        If `end` is specified then finding ends at this index.
        """
        inx = this[start..end].index(value)
        if (inx <= -1)
            return inx
        return inx + start
    }

    @internal
    fun insert(index:Int, value) {
        d"""
        Inserts `value` into this List before index `index`.
        If `index` > length of this List then `value` is appended at
        the end.
        If `index` <= 0 then `value` is prepended to the List.
        """
    }

    @internal
    fun length() {
        d"Return number of values in this list."
    }

    @internal
    fun pop(index:Int=-1) {
        d"""
        Removes and returns value from this list at index `index`.
        Might raise IndexError if `index` is out of bounds of this list.
        """
    }

    fun remove(value) {
        i = this.index(value)
        if (i < 0)
            raise ValueError(f"'{value}' is not present in List.\n")
        ~this.delete(i)
    }

    fun reverse() = this[-1..-this.length()-1]

    @internal
    fun __iter() {}

    @internal
    fun __next() {}
}

@internal_bind("Dict")
class __Dict {
    d"""
    A mutable, dynamically sized, unordered mapping type that associates
    hashable keys with arbitrary values.
    """

    @internal
    fun Dict() {
        d"Returns an empty dict."
    }
    @internal
    fun Dict(iterable) {
        d"""
        Returns a dict with pairs from iterable as its keys and values.
        `iterable` has to be a list of lists of size 2, if not then TypeError
        will be raised.
        Example:
        ```
        Dict([["a", 1], ["b", 2], ["c", 3]])
        // This will create {"a": 1, "b": 2, "c": 3}
        ```
        """
    }

    @internal
    fun clear() {
        d"Removes all value from this dictionary."
    }

    fun copy() = ::copy(this)

    fun empty() {
        d"""
        Returns true if there are no items in this dictionary, false otherwise.
        """
        return this.length() == 0
    }

    @static_method
    fun from_keys(iterable, value=nil) {
        d"""
        Creates a dictionary from `iterable` used as the keys and `value` as
        the value.
        """
        // TODO: Once length is fixed uncomment.
        // return ::Dict(zip(iterable, [value : _ = 0..length(iterable)]))
        d = {:}
        for (k: iterable) {
            d[k] = value
        }
        return d
    }

    @internal
    fun get(key, def_val=nil) {
        d"""
        Tries to extract and return `key` value from this dictionary, if it is
        not found, then def_val is returned.
        """
    }

    fun items() {
        d"Returns list of lists (pairs) from this dict."

        vals = []
        for (k, v: this) {
            ~vals.append([k, v])
        }
        return vals
    }

    fun keys() {
        d"Returns list of keys in this dict."

        ks = []
        for (k, _: this) {
            ~ks.append(k)
        }
        return ks
    }

    fun values() {
        d"Returns list of values in this dict."

        vls = []
        for (_, v: this) {
            ~vls.append(v)
        }
        return vls
    }

    @internal
    fun length() {
        d"Returns number of items (key and value) in this dictionary."
    }

    @internal
    fun pop(key) {
        d"""
        Removes and returns value at key `key`, if it is not found then KeyError
        is raised.
        """
    }

    @internal
    fun pop(key, def_val) {
        d"""
        Removes and returns value at key `key`, if it is not found then def_val
        is returned.
        """
    }

    fun pop_item() {
        d"Removed key and its value from this dict and returns it."
        
        if (this.empty())
            raise KeyError("Dictionary is empty in call to 'pop_item'.\n")
        items = this.items()
        last = items[-1]
        ~this.pop(last[0])
        return last
    }

    fun remove(key) {
        d"Removes item at key `key` from the dictionary."
        ~this.pop(key)
    }
}

@internal_bind("Bytes")
class __Bytes {
    d"""
    Represents linear sequence of bytes.
    """

    @internal
    fun Bytes(s:String) {
        d"""
        Converts `s` into byte representation.
        """
    }
}

@internal_bind("Range")
class __Range {
    d"""
    Represents arithmetic progression of integers.
    Range can be ascending or descending with variable size step.
    """

    fun Range(start:Int, end:Int, step:[Int,NilType]=nil) {
        d"""
        Creates range from `start` with step `step` (1 or -1 if nil) and end
        `end` (it is not included).
        If `end` is less than `start` and `step` is nil, then `step` is -1
        (range goes down).
        """
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
        d"""
        Creates range from 0 to `end` with step 1 or -1.
        """
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
    d"""
    Encloses an iterable in a new iterable with corresponding indices to values.
    The new values are pairs [index, value].
    """

    fun Enumerate(iterable, start:Int=0) {
        d"Creates an enumeration from iterable with starting index `start`."
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
        d"""
        Complex number with real and imaginary part.
        These parts might be Int or Float.
        """
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
    d"""
    Class for representing class type.
    The type of Type is itself (Type).
    """
    //@internal
    //fun Type() { }
}

@internal
fun vardump(value) {
    d"""
    Returns formatted internal info of `value` for outputting/inspecting.
    """
}

@internal
fun print(... msgs, end="\n", separator=" ") {
    d"""
    Outputs `msgs` values separated by `separator` and ended with `end`.
    Print is not outputting, it is not affected by formatters, generators nor
    converters.
    """
}

@internal
fun input(prompt="") {
    d"""
    Prints `prompt` to stdout and awaits user input from stdin.
    """
}

fun exit(code=0) {
    d"""
    Terminates this program with exit code `code`.
    This is internally done by raising SystemExit exception.
    """
    raise SystemExit(code)
}

@internal
fun round(n:[Int,Float], ndigits:[Int,NilType]=nil) {
    d"""
    Rounds `n` to number of digits `ndigits`.
    If `ndigits` is nil, then `n` is rounded to Int.
    When rounding using `Int`, then the decimal part is cut of, this function
    rounds.
    """
}

@internal
fun copy(obj) {
    d"""
    Does shallow copy of `obj`.
    """
}

@internal
fun hash(obj) {
    d"""
    Returns a hash of `obj`.
    If `obj` is an object then `__hash` method is called.
    """
}

@internal
fun type(obj) {
    d"""
    Returns type (class) of `obj`.
    """
}

@internal
fun id(obj) {
    d"""
    Returns unique id of `obj`.
    Note that some values, such as Ints might be interned and 2 separately
    created Ints might return same id.
    Note id of nil, true and false is always the same for each instance of these
    values in one run.
    """
}

@internal
fun isinstance(obj, types) {
    d"""
    Returns true if `obj` is instance of class `type` or one of the classes in
    list of classes `types`.
    Parameter `types` has to be a class or List of classes, otherwise TypeError
    will be raised.
    """
}

@internal
fun issubclass(cls:Type, types) {
    d"""
    Returns true if class `cls` is a subclass (inherits from) of class `type` or
    one of the classes in list of classes `types`.
    Parameter `types` has to be a class or List of classes, otherwise TypeError
    will be raised.
    """
}

@internal
fun attrs() {
    d"""
    Returns a list of names of symbols in the current scope.
    """
}
@internal
fun attrs(obj) {
    d"Returns a list of names of attributes in obj."
}

@internal
fun hasattr(obj, name:String) {
    d"Returns true if value `obj` contains attribute `name`."
}

@internal
fun getattr(obj, name:String) {
    d"Returns value of attribute `name` in value `obj`."
}

@internal
fun setattr(obj, name:String, value) {
    d"Sets attribute `name` in `obj` to value `value`."
}

@internal
fun delattr(obj, name:String) {
    d"""
    Deletes attribute `name` from `obj`.
    If `obj` is not modifiable or attribute does not exits, then AttributeError
    will be raised.
    """
}

@internal
fun locals() {
    d"""
    Returns dict of current scope symbols and their values.
    """
}

@internal
fun globals() {
    d"""
    Returns dict of global scope symbols and their values.
    """
}

@internal
fun callable(obj) {
    d"Returns true if `obj` can be called."
}

@internal
fun lshift(a:Int, count:Int) {
    d"""
    Returns `a` shifted left by the amount in `count`.
    """
}

@internal
fun rshift(a:Int, count:Int) {
    d"""
    Returns `a` shifted right by the amount in `count`.
    """
}

fun all(iterable, test:Function) {
    d"""
    Returns true if all values in `iterable` when passed into `test` return
    true.
    True will be returned also for empty iterable.
    """
    for (i : iterable) {
        if (not test(i))
            return false
    }
    return true
}

fun all(iterable) {
    d"""
    Returns true if all values in `iterable` are true.
    True will be returned also for empty iterable.
    """
    for (i: iterable) {
        if (not i)
            return false
    }
    return true
}

fun any(iterable, test:Function) {
    d"""
    Returns true if at least one value in `iterable` when passed into `test`
    returns true.
    True will be returned also for empty iterable.
    """
    for (i : iterable) {
        if (test(i))
            return true
    }
    return false
}

fun any(iterable) {
    d"""
    Returns true if at least one value in `iterable` is true.
    True will be returned also for empty iterable.
    """
    for (i: iterable) {
        if (i)
            return true
    }
    return false
}

fun length(iterable) {
    d"Returns amount of values in iterable."
    l = 0
    for (_: iterable)
        l += 1
    return l
}

// TODO: Change to not require [] operator
fun zip(... iterables) {
    d"""
    Takes one or more iterables and returns a iterable of lists, where the i-th
    list contains the i-th element from each input list. Stops when the
    shortest iterable is exhausted.
    """
    min_len = min([length(i) : i = iterables])
    return [[lst[i] : lst = iterables] : i = 0..min_len]
}

fun map(func, ... iterables) {
    d"""
    Make an iterator that computes the function using arguments from
    each of the iterables.  Stops when the shortest iterable is exhausted.
    """
    result = []
    for (args : zip(<<iterables)) {
        ~result.append(func(<<args))
    }
    return result
}

fun filter(func, iterable) {
    d"""
    Returns a list of those items in `iterable`, which returned true when
    passed into `func`.
    """
    result = []
    for (item : iterable) {
        if(func(item))
            ~result.append(item)
    }
    return result
}

space {
    enum ValType {
        MISSING,
        NONE
    }
}

// TODO: Can be optimized
fun max(iterable, key=nil, def_val=ValType.MISSING) {
    d"""
    Returns maximum value from iterable.
    If key is not nil, then it is used to compare values from iterable.
    When iterable is empty then def_val is returned if passed in, ValueError is
    raised otherwise.
    """
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
    d"""
    Returns minimum value from iterable.
    If key is not nil, then it is used to compare values from iterable.
    When iterable is empty then def_val is returned if passed in, ValueError is
    raised otherwise.
    """
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
fun chr(i:Int) {
    d"""
    Returns UNICODE string corresponding to integer value `i`.
    ValueError is raised if i < 0 or i > 0x10ffff.
    """
}

@internal
fun ord(c:String) {
    d"""
    Returns ordinal value of UNICODE symbol `c`.
    If c is more than 1 symbol, then ValueError is raised.
    """
}

@internal
fun hex(number:Int) {
    d"""
    Returns value of `number` as a hexadecimal string (hexstring).
    If `number` is negative then the hexstring will start with `-`.
    The hexstring returned is prefixed with `0x` and has no leading zeroes.
    """
}

@internal
fun oct(number:Int) {
    d"""
    Returns value of `number` as a octal string.
    If `number` is negative then the octal string will start with `-`.
    The octal string returned is prefixed with `0q` and has no leading zeroes.
    """
}

@internal
fun bin(number:Int) {
    d"""
    Returns value of `number` as a binary string.
    If `number` is negative then the binary string will start with `-`.
    The binary string returned is prefixed with `0b` and has no leading zeroes.
    """
}

fun info(v, indent:String="") {
    d"""
    Returns formatted doc string or list of pairs of [function, doc strings] for
    overloaded function.
    For multi-line doc string indentation is removed based on indentation of the
    first line.
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
    d"""
    Returns formatted doc string and additional type information of `a`.
    """
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
        en_name = inspect.name(a)
        txt ++= "enum " ++ en_name ++ " {"
        for (e: inspect.enum_values(a)) {
            txt ++= "\n        " ++ e
        }
        if (not inspect.enum_values(a).empty())
            txt ++= "\n    "
        txt ++= "}\n"
        txt ++= info(a, "    ")
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

/*fun help() {
    d"""
    Interactive help utility.
    This function queries for input and runs `help` on it.
    """
    ~print("""Moss interactive help utility.
------------------------------
You can enter a name of a module to get more information on it.
Currently this tool cannot work with expressions (e.g. "Int.Int").
Enter "q" or "quit" to exit this tool.
""")
}*/

@internal_bind("File")
class __File {
    d"""
    This class works with and represents system files.
    """

    fun File(path:String, mode:String="r") {
        d"""
        Initializes path and mode of operation with a file.
        Opening mode `mode` can be one of the following:
          - "r": reading.
          - "w": writing (overwrites a file if it exists).
          - "a": appending.
          - "rb": reading binary.
          - "wb": writing binary.
          - "ab": appending binary.
          - "r+": open/update (reading and writing).
          - "w+": writing/update.
          - "a+": append/update.
          - "r+b": reading/update binary.
          - "w+b": writing/update binary.
          - "a+b": append/update binary.
        When writing or appending to a file, which does not exist then it is
        created.
        When reading file, which does not exist, then FileNotFoundError is
        raised.
        """
        this.path = path
        this.mode = mode
    }

    @internal
    fun write(content) {
        d"""
        Writes content (converted to String if it is not already) to open file.
        """
    }

    @internal
    fun read(size:Int=-1) {
        d"""
        Reads `size` amount of bytes from this file.
        """
    }

    @internal
    fun readln(size:Int=-1) {
        d"""
        Reads one line of file.
        If `size` is specified then at most `size` bytes will be read.
        """
    }
    //fun seek(pos:Int) {}

    @internal
    fun readlines() {
        d"""
        Reads contents of the file and splits it by lines into a list, which it
        returns
        """
    }

    @internal
    fun open() {
        d"""
        Opens file.
        Open file has to be closed by calling close().
        """
    }

    @internal
    fun close() {
        d"""
        Closes file for all operations.
        """
    }
}

fun open(path:String, mode:String="r") {
    d"""
    Opens file on path `path` in mode `mode` and returns its File object.
    This file has to be explicitly closed.
    """
    f = File(path, mode)
    ~f.open()
    return f
}

fun with_open(path:String, func, mode:String="r") {
    d"""
    Opens file on path `path` in mode `mode` and passes it into `func`, closes 
    the file and returns value from `func`.
    """
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
fun rand_int(min:Int, max:Int) {
    d"""
    Returns a pseudo-random integer with uniform distribution from `min` to
    `max` (both including).
    If min > max, then ValueError is raised.
    """
}

@internal
fun rand_float(min:[Float,Int]=0.0, max:[Float,Int]=1.0) {
    d"""
    Returns a pseudo-random float with uniform distribution from `min` to
    `max` (both including).
    If min > max, then ValueError is raised.
    """
}

@internal
fun sleep(seconds:[Float,Int]) {
    d"""
    Puts this thread into sleep for `seconds` time.
    `seconds` can be floating point representing a part of a second.
    """
}

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
        Returns sine of `x`.
        """
    }

    @internal
    fun tan(x:[Int,Float]) {
        d"""
        Returns tangent of `x`.
        """
    }

    @internal
    fun cos(x:[Int,Float]) {
        d"""
        Returns cosine of `x`.
        """
    }

    fun sqrt(x) {
        d"""
        Returns square root of `x`.
        This is equal to `x^0.5`.
        """
        return x^0.5;
    }

    @internal
    fun abs(n:[Int,Float]) {
        d"""
        Returns absolute value of `n`.
        """
    }

    fun sum(nums, start:[Int,Float]=0) {
        d"""
        Returns sum of values on `nums`, if `start` is set then it will be used
        as the starting value for the sum.
        """
        for (i : nums) {
            start += i
        }
        return start
    }

    fun cumsum(nums) {
        d"Returns cumulative sum of value in `nums`."
        total = 0
        accum = []
        for (v : nums) {
            total += v
            ~accum.append(total)
        }
        return accum
    }

    @internal
    fun log(x:[Int,Float], base:[Int,Float]) {
        d"Returns logarithm of base `base` of `x`."
    }

    @internal
    fun divmod(x:[Int,Float], y:[Int,Float]) {
        d"""
        Divides `x` by `y` and returns the result and reminder as a pair of
        values (list of size 2).
        """
    }
}

space Converters {
d"""
Space for converters for moss known formats.
These converters should provide basic file generation.

These can be easily overridden by custom converters in user scope.
"""

@converter("txt", "md")
@converter("txt", "markdown")
fun txt2md(n) {
    d"Converter from text format to markdown format."

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

@converter("csv", "md")
@converter("csv", "markdown")
fun csv2md(n) {
    d"Converter from csv format to markdown format (table)."

    import csv_parser
    parser = csv_parser.CSVParser(n)
    return parser.to_md()
}

@converter("txt", "html")
@converter("txt", "html5")
@converter("txt", "html4")
fun txt2html(txt) {
    d"Converter from text format to html format."

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
    d"Converter from markdown to html format."

    import md_parser
    parsed = md_parser.parse_md(txt)
    converted = [l.to_html() : l = parsed]
    return "\n".join([String(x) : x = converted])
}

@converter("csv", "html")
fun csv2html(txt) {
    d"Converter from csv to html format (table)."

    import csv_parser
    parser = csv_parser.CSVParser(txt)
    return parser.to_html()
}

}

space Generators {
d"""
Space for generators for moss known formats.
These generators should provide basic file generation.

These can be easily overridden by custom generators in user scope.
"""

space HTML {
    d"""
    This space holds constants used for HTML generation and conversion.
    """
    STYLE_PATH="" //< This value will be set based on the platform 
    TITLE=""
    FOOTER=nil
}

@generator("html")
fun html_gen(txts) {
    d"""
    HTML generator. This generator uses css style (specified by HTML.STYLE_PATH)
    to style the generated document.
    """

    import sys.version_info
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
        footer = f"""Page generated on {strftime("%b %d %Y")} by <a href="https://github.com/mark-sed/moss-lang">Moss</a> {version_info.major++"."++version_info.minor++"."++version_info.patch}."""
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
    d"""
    Base class from Moss exceptions.
    Each exception holds string `msg`, which should describe the exception
    further in detail.
    """

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

@internal_bind("OutputError")
class __OutputError : Exception {
    fun __String() {
        return "OutputError: " ++ this.msg
    }
}

@internal_bind("SystemExit")
class __SystemExit : Exception {
    d"""
    Exception representing termination of a program.
    If not caught Moss will terminate with exit code passed in.
    This exception is used by `exit` function.
    """

    fun SystemExit(code=0) {
        this.msg = this.code = code
    }

    fun __String() {
        return String(this.code)
    }
}