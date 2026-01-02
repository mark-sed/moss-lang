d"""
Module for working with regular expressions.
"""

class REFlags {
    enum Grammars {
        ECMAScript
        basic
        extended
        awk
        grep
        egrep
    }

    fun REFlags(grammar:Grammars=Grammars.ECMAScript,
                icase:Bool=false,
                nosubs:Bool=false,
                optimize:Bool=false,
                collate:Bool=false) {
        this.grammar = grammar
        this.icase = icase
        this.nosubs = nosubs
        this.optimize = optimize
        this.collate = collate
    }
}

DEFAULT_FLAGS = REFlags()

class Pattern {
    d"""
    Represents regular expression pattern.
    """

    @internal
    fun Pattern(pattern:String, flags:REFlags=DEFAULT_FLAGS) {}

    @internal
    fun match(text:String) {}

    @internal
    fun search(text:String) {}
}

class Match {
    d"""
    Class for returning results of sucessfull match and search.
    """

    fun Match(pattern:Pattern, text:String, groups:List) {
        this.pattern = pattern
        this.text = text
        this.groups = groups
    }

    fun ([])(index:Int) {
        return this.groups[index]
    }

    fun __Bool() {
        return true
    }
}


fun compile(pattern:String, flags:REFlags=DEFAULT_FLAGS) {
    d"""
    Compiles a new regular expression object.
    """
    return Pattern(pattern, flags)
}

fun match(pattern:String, text:String, flags:REFlags=DEFAULT_FLAGS) {
    d"""
    Tries to match `pattern` regular expression to the entire String `text`.
    This returns `Match` object if matched or nil if not.
    """
    r = compile(pattern, flags)
    return r.match(text)
}

fun search(pattern:String, text:String, flags:REFlags=DEFAULT_FLAGS) {
    d"""
    Tries to match `pattern` regular expression to any part of String `text`.
    This returns `Match` object if matched or nil if not.
    """
    r = compile(pattern, flags)
    return r.search(text)
}