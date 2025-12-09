d"""
Module for parsing, converting and manipulation with CSV (Comma-separated-
values) files.
This parser is trying to be in accordance to RFC4180, but also allow for
custom quality of life features.
"""

import parsing_utils

space Dialects {
    enum Quoting {
        MINIMAL     // Quoted if the field contains delimiter, quote_char, newlines or trailing/leading whitespace.
        ALL         // All values are wrapped in quote_char.
        NON_NUMERIC // Non-quoted fields are parsed as Float.
    }

    class Dialect {
        fun Dialect(delimiter:String = ",",
                    quote_char:String = "\"",
                    escape_char:String = "\\",
                    double_quote:Bool = true,
                    skip_initial_space:Bool = false,
                    line_terminator:String = "\n",
                    quoting:Dialects.Quoting = Quoting.MINIMAL,
                    strict:Bool = false) {
            this.delimiter = delimiter
            this.quote_char = quote_char
            this.escape_char = escape_char
            this.double_quote = double_quote
            this.skip_initial_space = skip_initial_space
            this.line_terminator = line_terminator
            this.quoting = quoting
            this.strict = strict
        }
    }

    // TODO: Add pre-made dialects
}

space {
    __DEFAULT_DIALECT = Dialects.Dialect()
}

class CSVError : Exception {
    fun CSVError(line:Int, column:Int, text:String) {
        ~super(text)
        this.line = line
        this.column = column
        this.text = text
    }

    fun __String() {
        return f"CSVError:{this.line}:{this.column}: {this.text}.\n"
    }
}

class CSVParser : parsing_utils.TextualParser {

    fun CSVParser(text:String, dialect:Dialects.Dialect=__DEFAULT_DIALECT) {
        ~super(text)
        this.dialect = dialect
    }

    fun parse() {
        record_length = nil
        this.line = 1
        this.column = 1
        delim_len = this.dialect.delimiter.length()

        fun append_value() {
            str_v = "".join($value)
            if (this.dialect.quoting == Dialects.Quoting.NON_NUMERIC && not $value.empty() && $value[0] != "\"") {
                ~$record.append(Float(str_v))
            } else {
                if ($value.length() > 1) {
                    if ($value[0] == "\"" && $value[-1] == "\"")
                        str_v = str_v[1..str_v.length()-1]
                }
                ~$record.append(str_v)
            }
            this.column += $delim_len
            $value = []
        }

        csv = []
        record = []
        value = []
        in_quote = false
        while (this.i < this.text.length()) {
            // Skipping starting spaces
            while (this.dialect.skip_initial_space &&
                    (this.check_adv(" ") || this.check_adv("\t"))) {
                if (this.i >= this.text.length())
                    break
                this.column += 1
            }
            // new line
            if (not in_quote && this.check_adv(this.dialect.line_terminator)) {
                ~append_value()
                ~csv.append(record)
                // Check for correct number of values in a record
                if (this.dialect.strict && record_length != nil && record.length() != record_length) {
                    raise CSVError(this.line, this.column, "Varying number of values in records")
                }
                if (record_length == nil)
                    record_length = record.length()
                record = []
                this.line += 1
                this.column = 1
            } else if (not in_quote && this.check_adv(this.dialect.delimiter)) {
                ~append_value()
            } else {
                c = this.advance()
                ~value.append(c)
            }
        }

        if (this.dialect.strict && in_quote) {
            raise CSVError(this.line, this.column, "Unterminated quote (\")")
        }

        return csv
    }
}

fun list_reader(s:String, dialect:Dialects.Dialect=__DEFAULT_DIALECT) {
    d"""
    Reads csv String `s` and returns it as a List of Lists, where the elements
    are the parsed and trimmed csv values.
    """
    parser = CSVParser(s, dialect)
    return parser.parse()
}

fun list_reader(f:File, dialect:Dialects.Dialect=__DEFAULT_DIALECT) {
    d"""
    Reads csv file `f` and returns it as a List of Lists, where the elements are
    the parsed and trimmed csv values.
    """
    return list_reader(f.read(), dialect)
}

fun dict_reader(f:File, header:[NilType,List]=nil, dialect:Dialects.Dialect=__DEFAULT_DIALECT) {
    d"""
    Reads passed in csv file `f` and returns it as a list of dict values.
    Names (keys) for columns will be read in the first line unless `header`
    is specified (List of Strings).
    """

}