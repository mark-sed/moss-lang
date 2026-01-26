d"""
Module for parsing JSON format.
"""

import parsing_utils

class JSONParsingError : Exception {
    d"""
    Exception used by JSON parser when an error is encountered.
    """

    fun JSONParsingError(line:Int, column:Int, text:String) {
        d"""
        Constructs JSON parsing exception.
        Parameter `line` and `column` correspond to the location where this
        error occurred.
        Parameter `text` is the error message to display.
        """
        ~super(text)
        this.line = line
        this.column = column
        this.text = text
    }

    fun __String() {
        if (this.line <= 0 && this.column <= 0)
            return f"JSONParsingError: {this.text}.\n"
        return f"JSONParsingError:{this.line}:{this.column}: {this.text}.\n"
    }
}

class JSONParser : parsing_utils.TextualParser {

    fun JSONParser(text:String) {
        ~super(text)
    }

    fun expect(s:String, error:String) {
        if (not this.match(s)) {
            raise JSONParsingError(this.line, this.column, error)
        }
        // Match increments column
    }

    fun match(s:String) {
        if (this.check_adv(s)) {
            this.column += 1
            return true
        }
        return false
    }

    fun advance() {
        v = super.advance()
        // Wont be called on "\n" so just column increase is ok.
        this.column += 1
        return v
    }

    fun skip_json_ws() {
        while (this.i < this.text.length()) {
            // Skip new lines and ws
            if (this.match(" ") || this.match("\t") || this.match("\r")) {
                this.column += 1
                continue
            }
            if (this.match("\n")) {
                this.column = 1
                this.line += 1
                continue
            }
            break
        }
    }

    fun parse_string() {
        value = []
        terminated = false
        while (this.i < this.text.length()) {
            if (this.match("\"")) {
                terminated = true
                break
            }
            if (this.check("\n")) {
                break
            }
            if (this.match("\\")) {
                if (this.i >= this.text.length()) {
                    raise JSONParsingError(this.line, this.column, "Unterminated escape sequence ('\\')")
                }
                c = this.advance()
                switch (c) {
                    case "\"": ~value.append("\"")
                    case "\\": ~value.append("\\")
                    case "/": ~value.append("/")
                    case "b": ~value.append("\b")
                    case "f": ~value.append("\f")
                    case "n": ~value.append("\n")
                    case "r": ~value.append("\r")
                    case "t": ~value.append("\t")
                    case "u": assert(false, "TODO: IMPLEMENT hex escape sequence") // TODO
                    default: raise JSONParsingError(this.line, this.column, f"Unknown escape sequence '\\{c}'")
                }
                continue
            }
            ~value.append(this.advance())
        }
        if (not terminated) {
            raise JSONParsingError(this.line, this.column, "Unterminated string (expected '\"')")
        }
        return "".join(value)
    }

    fun parse_object() {
        result = {:}
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            ok = false
            switch (c) {
                case "}": {
                    ~this.advance()
                    break // closing brace
                }
                case " ", "\t", "\r": {
                    ~this.advance()
                    continue
                }
                case "\n": {
                    this.line += 1
                    this.column = 1
                    this.i += 1
                    continue
                }
                case "\"": {
                    ~this.advance()
                    // Member
                    name = this.parse_string()
                    ~this.skip_json_ws()
                    ~this.expect(":", "Expecting ':' after member name")
                    ~this.skip_json_ws()
                    //value =

                    // TODO: Finish

                    continue
                }
            }
            raise JSONParsingError(this.line, this.column, f"Expected member name or '}}', but found '{this.text[this.i]}'")
        }
        return result
    }

    // TODO: Support \uXXXX escape sequence to comply with rfc
    fun parse() {
        this.line = 1
        this.column = 1
        result = {:}
        ~this.skip_json_ws()
        while (this.i < this.text.length()) {
            // Starting '{' match
            ~this.expect("{", "JSON must start with white space or '{'") // "}"
            result = this.parse_object()
            ~this.skip_json_ws()
            if (this.i < this.text.length()) {
                // If reached here it means that there are some symbols after main
                // object and it is not white-space
                raise JSONParsingError(this.line, this.column, 
                        "Unexpected non-whitespace character after JSON")
            }
            // No else need as while will break for else condition.
        }
        return result
    }

}

fun dict_reader(s:String) {
    d"""
    Parses passed in JSON string `s` and returns it as a corresponding Dict.
    """
    parser = JSONParser(s)
    return parser.parse()
}

fun dict_reader(f:File) {
    d"""
    Reads passed in JSON file `f` and returns it as a corresponding Dict.
    """
    s = f.read()
    return ::dict_reader(String(s))
}