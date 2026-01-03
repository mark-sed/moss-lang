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
                collate:Bool=false,
                multiline:Bool=false,
                match_default:Bool=false,
                match_not_bol:Bool=false,
                match_not_eol:Bool=false,
                match_not_bow:Bool=false,
                match_not_eow:Bool=false,
                match_any:Bool=false,
                match_not_null:Bool=false,
                match_continuous:Bool=false,
                match_prev_avail:Bool=false,
                format_default:Bool=false,
                format_sed:Bool=false,
                format_no_copy:Bool=false,
                format_first_only:Bool=false) {
        // Syntax flags
        this.grammar = grammar
        this.icase = icase
        this.nosubs = nosubs
        this.optimize = optimize
        this.collate = collate
        this.multiline = multiline
        // Runtime flags
        this.match_default = match_default
        this.match_not_bol = match_not_bol
        this.match_not_eol = match_not_eol
        this.match_not_bow = match_not_bow
        this.match_not_eow = match_not_eow
        this.match_any = match_any
        this.match_not_null = match_not_null
        this.match_continuous = match_continuous
        this.match_prev_avail = match_prev_avail
        this.format_default = format_default
        this.format_sed = format_sed
        this.format_no_copy = format_no_copy
        this.format_first_only = format_first_only
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