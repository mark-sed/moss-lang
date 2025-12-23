d"""
Module for parsing, converting and manipulation with CSV (Comma-separated-
values) files.
This parser is trying to be in accordance with RFC4180, but also allow for
custom quality of life features.
"""

import parsing_utils
import sys

class CSVError : Exception {
    d"""
    Exception used by csv_parser when an error is encountered.
    If this error is parsing error then the corresponding line and column,
    where this error occurred will be in `line` and `column` attributes. 
    """

    fun CSVError(line:Int, column:Int, text:String) {
        d"""
        Constructs CSV exception.
        Parameter `line` and `column` correspond to the location where this
        error occurred if it is a parsing error, if not then these should be 0.
        Parameter `text` is the error message to display.
        """
        ~super(text)
        this.line = line
        this.column = column
        this.text = text
    }

    fun __String() {
        if (this.line <= 0 && this.column <= 0)
            return f"CSVError: {this.text}.\n"
        return f"CSVError:{this.line}:{this.column}: {this.text}.\n"
    }
}

space Dialects {
    d"""
    Contains resources related to CSV dialects.
    Dialects control how CSV parsing and writing is done since CSV is not
    truly standardized and different programs use different dialects.

    This space also contains pre-set dialects:
    - DEFAULT - In accordance with RFC4180 but with LF as line_terminator.
    - EXCEL - For comma separated CR LF CSVs from Microsoft Excel.
    - EXCEL_TAB - For tab separated CR LF CSVs from Microsoft Excel.
    """

    enum Quoting {
        d"""
        Controls how/when values should be quoted.
        - MINIMAL     - Quoted if the field contains delimiter, quote_char,
                        newlines or trailing/leading whitespace.
        - ALL         - All values are wrapped in quote_char.
        - NON_NUMERIC - Non-quoted fields are parsed as Float.
        """
        MINIMAL     // Quoted if the field contains delimiter, quote_char, newlines or trailing/leading whitespace.
        ALL         // All values are wrapped in quote_char.
        NON_NUMERIC // Non-quoted fields are parsed as Float.
    }

    class Dialect {
        d"""
        Represents CSV dialect with all the attributes that can control parsing
        and writing of CSV.
        To create a new dialect just instantiate this class and set attributes
        you wish to change from default.
        Example:
        ```
        Dialects.Dialect(delimiter=" ", strict=true)
        ```
        """

        fun Dialect(delimiter:String = ",",
                    quote_char:String = "\"",
                    escape_char:String = "\\",
                    double_quote:Bool = true,
                    skip_initial_space:Bool = false,
                    line_terminator:String = "\n",
                    quoting:Dialects.Quoting = Quoting.MINIMAL,
                    strict:Bool = false) {
            d"""
            Constructs a new dialect.
            Parameter `delimiter`, `quote_char` and `escape_char` must be 1
            character long strings otherwise CSVError is raised.
            """
            this.delimiter = delimiter
            this.quote_char = quote_char
            this.escape_char = escape_char
            this.double_quote = double_quote
            this.skip_initial_space = skip_initial_space
            this.line_terminator = line_terminator
            this.quoting = quoting
            this.strict = strict
            if (delimiter.length() != 1) {
                raise CSVError(0, 0, f"Delimiter ('Dialects.delimiter') must be 1 character long ('{delimiter}' is {delimiter.length()})")
            }
            if (quote_char.length() != 1) {
                raise CSVError(0, 0, f"Quote character ('Dialects.quote_char') must be 1 character long ('{quote_char}' is {quote_char.length()})")
            }
            if (escape_char.length() != 1) {
                raise CSVError(0, 0, f"Escape character ('Dialects.escape_char') must be 1 character long ('{escape_char}' is {escape_char.length()})")
            }
        }
    }
    
    // TODO: Handle somehow 
    DEFAULT = Dialects.Dialect() // Dialect with default arguments. Used when dialect is not specified.
    
    EXCEL = Dialects.Dialect(
        delimiter=",",
        quote_char="\"",
        escape_char="\\",
        double_quote=true,
        skip_initial_space=false,
        line_terminator="\r\n",
        quoting=Quoting.MINIMAL
    )
    EXCEL_TAB = Dialects.Dialect(
        delimiter="\t",
        quote_char="\"",
        escape_char="\\",
        double_quote=true,
        skip_initial_space=false,
        line_terminator="\r\n",
        quoting=Quoting.MINIMAL
    )
}

class CSVParser : parsing_utils.TextualParser {

    fun CSVParser(text:String, dialect:Dialects.Dialect=Dialects.DEFAULT) {
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
        // Skip new lines at the start of the file - line_terminator can be multiple chars
        while (this.i < this.text.length() && this.check_adv_str(this.dialect.line_terminator)) {
            this.line += 1
        }
        // Remove new lines at the end
        while (not this.text.empty() &&
                this.text[this.text.length()-this.dialect.line_terminator.length()..this.text.length()] == this.dialect.line_terminator) {
            this.text = this.text[0..this.text.length()-this.dialect.line_terminator.length()]
        }
        was_parsing = false
        while (this.i < this.text.length()) {
            // Skipping starting spaces
            while (this.dialect.skip_initial_space &&
                    (this.check_adv(" ") || this.check_adv("\t"))) {
                this.column += 1
                if (this.i >= this.text.length())
                    break
            }
            was_parsing = true
            // new line
            if (not in_quote && this.check_adv_str(this.dialect.line_terminator)) {
                if (not (record.empty() && value.empty()))
                    ~append_value()
                ~csv.append(record)
                // Check for correct number of values in a record
                if (this.dialect.strict && record_length != nil && record.length() != record_length) {
                    raise CSVError(this.line, this.column, f"Varying number of values in records (from {record_length} to {record.length()})")
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
                if (c == this.dialect.quote_char) {
                    if (in_quote && this.dialect.double_quote && this.check_adv(this.dialect.quote_char)) {
                        // "" double quote - escaped, just let c be appended later on
                    } else
                        in_quote = not in_quote
                } else if (not this.dialect.double_quote && c == this.dialect.escape_char && this.check_adv(this.dialect.quote_char)) {
                    // Escaped quote
                    this.column += 1
                    c = this.dialect.quote_char
                }
                ~value.append(c)
                this.column += 1
            }
        }

        if (this.dialect.strict && in_quote) {
            raise CSVError(this.line, this.column, f"Unterminated quote ('{this.dialect.quote_char}')")
        }

        if (was_parsing) {
            ~append_value()
            ~csv.append(record)
            // Check for correct number of values in a record
            if (this.dialect.strict && record_length != nil && record.length() != record_length) {
                raise CSVError(this.line, this.column, f"Varying number of values in records (from {record_length} to {record.length()})")
            }
        }

        return csv
    }
}

fun list_reader(s:String, dialect:Dialects.Dialect=Dialects.DEFAULT) {
    d"""
    Reads csv String `s` and returns it as a List of Lists, where the elements
    are the parsed and trimmed csv values.
    """
    parser = CSVParser(s, dialect)
    return parser.parse()
}

fun list_reader(f:File, dialect:Dialects.Dialect=Dialects.DEFAULT) {
    d"""
    Reads csv file `f` and returns it as a List of Lists, where the elements are
    the parsed and trimmed csv values.
    """
    s = f.read()
    parser = CSVParser(s, dialect)
    return parser.parse()
}

fun dict_reader(s:String, header:[NilType,List]=nil, dialect:Dialects.Dialect=Dialects.DEFAULT) {
    d"""
    Parses passed in csv string `s` and returns it as a list of dict values.
    Names (keys) for columns will be read in the first line unless `header`
    is specified (List of Strings).
    """
    parser = CSVParser(s, dialect)
    lst = parser.parse()
    if (lst.empty())
        return {:}
    start = 0
    if (header == nil) {
        header = lst[0]
        start = 1
    }
    res = []
    for (i : start..lst.length()) {
        l = lst[i]
        dct = {:}
        assert(l.length() == header.length(), "Header size does not match amount of values")
        for (j: 0..l.length()) {
            dct[header[j]] = l[j]
        }
        ~res.append(dct)
    }
    return res
}

fun dict_reader(f:File, header:[NilType,List]=nil, dialect:Dialects.Dialect=Dialects.DEFAULT) {
    d"""
    Reads passed in csv file `f` and returns it as a list of dict values.
    Names (keys) for columns will be read in the first line unless `header`
    is specified (List of Strings).
    """
    s = f.read()
    return ::dict_reader(String(s), header, dialect)
}