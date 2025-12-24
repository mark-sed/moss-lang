d"""
Module for parsing markdown for conversions to other formats.
"""

import parsing_utils
import html_parser

class MDToken {

    enum TokenType {
        TEXT
        LINK
        HEADING
        PARAGRAPH
        BLOCK_QUOTES
        LIST
        CODE_BLOCK
        HLINE
        TABLE // TODO: Implement table
    }

    fun MDToken(value, type:TokenType) {
        this.value = value
        this.type = type
    }

    fun to_txt() {
        return this.value
    }

    fun to_html() {
        assert(false, "Called to_html to pure MDToken or to_html is not overriden")
        return this.value
    }

    fun __String() {
        return String(this.value)
    }
}

class MDText : MDToken {

    // TODO: Autolink
    // TODO: Reference-style link
    fun MDText(text:String, 
               italic:Bool=false,
               bold:Bool=false,
               code:Bool=false,
               strikethrough:Bool=false) { 
        ~super(text, MDToken.TokenType.TEXT)
        this.italic = italic
        this.bold = bold
        this.code = code
        this.strikethrough = strikethrough
    }

    fun to_txt() {
        return this.value
    }

    fun to_html() {
        return html_parser.HTMLText(this.value, this.italic, this.bold, this.code, this.strikethrough)
    }

    fun __String() {
        // TODO: Escape values that need escaping
        text = this.value
        if (this.code)
            text = "`" ++ text ++ "`"
        if (this.italic)
            text = "_" ++ text ++ "_"
        if (this.bold)
            text = "**" ++ text ++ "**"
        if (this.strikethrough)
            text = "~~" ++ text ++ "~~"
        return text
    }
}

class MDLink : MDToken {
    fun MDLink(text, url:String, title:String, image:Bool) {
        ~super(url, MDToken.TokenType.LINK)
        this.text = text
        this.url = url
        this.title = title
        this.image = image
    }

    fun to_txt() {
        return this.url
    }

    fun to_html() {
        if (this.image)
            return html_parser.HTMLImage([x.to_html() : x = this.text], this.url, this.title)
        return html_parser.HTMLLink([x.to_html() : x = this.text], this.url, this.title)
    }

    fun __String() {
        return (this.image ? "![" : "[") ++ ("".join([String(x) : x = this.text])) ++ "](" ++ this.url ++ "\"" ++ this.title ++ "\")"
    }
}

class MDHeading : MDToken {

    fun MDHeading(value, level:Int) {
        ~super(value, MDToken.TokenType.HEADING)
        this.level = level
    }

    fun to_txt() {
        return "".join([x.to_txt() : x = this.value])
    }

    fun to_html() {
        return html_parser.HTMLHeading([x.to_html() : x = this.value], this.level)
    }

    fun __String() {
        return ("#" * this.level) ++ " " ++ ("".join([String(x) : x = this.value]))
    }
}

class MDParagraph : MDToken {
    fun MDParagraph(value) {
        ~super(value, MDToken.TokenType.PARAGRAPH)
    }

    fun to_txt() {
        return "".join([x.to_txt() : x = this.value])
    }

    fun to_html() {
        return html_parser.HTMLParagraph([x.to_html() : x = this.value])
    }

    fun __String() {
        return "".join([String(x) : x = this.value])
    }
}

class MDCodeBlock : MDToken {
    fun MDCodeBlock(value, format:String) {
        ~super(value, MDToken.TokenType.CODE_BLOCK)
        this.format = format
    }

    fun to_txt() {
        return "\n"++this.value
    }

    fun to_html() {
        return html_parser.HTMLCodeBlock(this.value, this.format)
    }

    fun __String() {
        return f"""```{this.format}
{this.value}```"""
    }
}

class MDBlockQuotes : MDToken {
    fun MDBlockQuotes(value, level) {
        ~super(value, MDToken.TokenType.BLOCK_QUOTES)
        this.level = level
    }

    fun to_txt() {
        return "".join([x.to_txt() : x = this.value])
    }

    fun to_html() {
        return html_parser.HTMLBlockQuotes([x.to_html() : x = this.value], this.level)
    }

    fun __String() {
        return (">"*this.level) ++ ("".join([String(x) : x = this.value]))
    }
}

class MDList : MDToken {
    // TODO: Handle task list
    fun MDList(values, ordered:Bool, bullet:[String,Int]) {
        ~super(values, MDToken.TokenType.LIST)
        this.ordered = ordered
        this.bullet = bullet
        assert(not ordered || type(bullet) == Int, "Ordered list but bullet is not a number (" ++ bullet ++ ")")
    }

    fun indented(bullet, level:Int=0) {
        t = ""
        for (v: this.value) {
            if (type(v) == ::MDList) {
                t ++= v.indented(v.bullet, level+1)
            } else {
                t ++= ("  " * level) ++ bullet ++ (this.ordered ? ". " : " ") ++ v ++ "\n"
                if (this.ordered)
                    bullet += 1
            }
        }
        return t
    }

    fun to_txt() {
        // TODO: Maybe force only 1 bullet type (-)?
        return this.indented(this.bullet)
    }

    fun to_html() {
        if (this.ordered)
            return html_parser.HTMLList([x.to_html() : x = this.value], this.ordered, this.bullet)
        else
            return html_parser.HTMLList([x.to_html() : x = this.value], this.ordered)
    }

    fun __String() {
        return this.indented(this.bullet)
    }
}

class MDHLine : MDToken {
    fun MDHLine(text) {
        ~super(text, MDToken.TokenType.HLINE)
    }

    fun to_txt() {
        return this.value
    }

    fun to_html() {
        return html_parser.HTMLHorizontalRule()
    }

    fun __String() {
        return this.value
    }
}

class MDTable : MDToken {
    enum CELL_ALIGNMENT {
        DEFAULT
        LEFT
        CENTER
        RIGHT
    }

    fun MDTable(table:List, has_header=false, alignments:List=[]) {
        ~super(table, MDToken.TokenType.TABLE)
        this.alignments = alignments
        this.has_header = has_header
    }

    fun to_txt() {
        t = ""
        header = true
        for (r: this.value) {
            t ++= "|"
            for (e: r) {
                for (me: e)
                    t ++= me.to_txt() ++ "|"
            }
            t ++= "\n"
            if (header && this.has_header) {
                header = false
                t ++= "|"
                for (aln: this.alignments) {
                    switch(aln) {
                        case CELL_ALIGNMENT.DEFAULT: t ++= "---"
                        case CELL_ALIGNMENT.LEFT:    t ++= ":---"
                        case CELL_ALIGNMENT.CENTER:  t ++= ":---:"
                        case CELL_ALIGNMENT.RIGHT:   t ++= "---:"
                    }
                    t ++= "|"
                }
                t ++= "\n"
            }
        }
        return t
    }

    fun to_html() {
        lst = []
        for (row: this.value) {
            vs = []
            for (cell: row) {
                cs = []
                for (v: cell) {
                    if (v.type == MDToken.TokenType.PARAGRAPH) {
                        ~cs.append("".join([x.to_html() : x = v.value]))
                    } else
                        ~cs.append(v.to_html())
                }
                ~vs.append(cs)
            }
            ~lst.append(vs)
        }
        if (this.alignments != nil) {
            align = []
            for (aln: this.alignments) {
                switch(aln) {
                    case CELL_ALIGNMENT.DEFAULT: ~align.append(html_parser.HTMLTable.CELL_ALIGNMENT.DEFAULT)
                    case CELL_ALIGNMENT.LEFT:    ~align.append(html_parser.HTMLTable.CELL_ALIGNMENT.LEFT)
                    case CELL_ALIGNMENT.CENTER:  ~align.append(html_parser.HTMLTable.CELL_ALIGNMENT.CENTER)
                    case CELL_ALIGNMENT.RIGHT:   ~align.append(html_parser.HTMLTable.CELL_ALIGNMENT.RIGHT)
                    default: assert(false, "Missing alignment case")
                }
            }
            return html_parser.HTMLTable(lst, alignments=align)
        } 
        return html_parser.HTMLTable(lst)
    }

    fun __String() {
        return this.to_txt()
    }
}

class MDParser : parsing_utils.TextualParser {

    fun MDParser(text:String, allow_strikethrough:Bool=true) {
        ~super(text)
        this.allow_strikethrough = allow_strikethrough
        if (this.allow_strikethrough)
            this.ESCAPE_LIST = "`*_[]()#+-.!>\\~"
        else
            this.ESCAPE_LIST = "`*_[]()#+-.!>\\"
    }

    fun compact_spaces() {
        while((this.i+1 < this.text.length()) && (this.check(" ") || this.check("\t")) && (this.text[this.i+1] == " " || this.text[this.i+1] == "\t")) {
            this.i += 1
        }
    }
    
    fun link() {
        fun cancel(image:Bool, text=nil, url=nil, title=nil) {
            result = image ? [MDText("!")] : []
            if (text != nil)
                result += [MDText("[")] + text
            if (url != nil)
                result += [MDText("]("++"".join(url))]
            if (title != nil)
                result += [MDText("\""++"".join(title))]
            return result
        }
        image = this.check_adv("!")
        assert(this.text[this.i] == "[", "Called link parsing on non-link text -- does not start with [")
        this.i += 1
        text = this.textual("]")
        if (not this.check_adv("]")) {
            return cancel(image, text)
        }
        if (not this.check_adv("(")) {
            return cancel(image, text+[MDText("]")])
        }
        url = []
        while (this.i < this.text.length() && not this.text[this.i] in "\")\n") {
            ~url.append(this.text[this.i])
            this.i += 1
        }
        if (this.check("\n")) {
            return cancel(image, text, url)
        }
        if (this.check_adv(")")) {
            return [MDLink(text, "".join(url), "", image)]
        }
        title = []
        if (this.check_adv("\"")) {
            while (this.i < this.text.length() && not this.check("\"") && not this.check("\n")) {
                if (this.check_adv("\\")) {
                    // Possible escape of "
                    if (this.check_adv("\"")) {
                        ~title.append("\"")
                    } else {
                        ~title.append("\\")
                    }
                } else {
                    ~title.append(this.text[this.i])
                    this.i += 1
                }
            }
        }
        if (not this.check_adv("\"")) {
            return cancel(image, text, url, title)
        }
        ~this.skip_ws()
        if (this.check_adv(")")) // Last enclosing of link
            return [MDLink(text, "".join(url), "".join(title), image)]
        // Something else than ), so returning as a text
        return cancel(image, text, url, title+["\""])
    }

    fun textual(end_on="") {
        fun add_text() {
            if (elem.length() == 0)
                return
            ~$toks.append(MDText("".join($elem), italic=this.italic or this.italic_star, bold=this.bold or this.bold_star, code=this.code, strikethrough=this.strikethrough))
            $elem = []
        }
        elem = []
        toks = []
        escape = false
        og_text = []
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            ~og_text.append(c)
            if (escape && c in this.ESCAPE_LIST) {
                ~elem.append(this.text[this.i])
                this.i += 1
                escape = false
            } else if (this.check_adv("`")) {
                ~add_text()
                this.code = not this.code
            } else if (not this.code && this.check_adv("_")) {
                if (this.check_adv("_")) {
                    ~og_text.append("_")
                    if (this.check_adv("_")) {
                        ~og_text.append("_")
                        ~add_text()
                        this.bold = not this.bold
                        this.italic = not this.italic
                    } else {
                        ~add_text()
                        this.bold = not this.bold
                    }
                } else {
                    ~add_text()
                    this.italic = not this.italic
                }
            } else if (not this.code && this.check_adv("*")) {
                if (this.check_adv("*")) {
                    ~og_text.append("*")
                    if (this.check_adv("*")) {
                        ~og_text.append("*")
                        ~add_text()
                        this.bold_star = not this.bold_star
                        this.italic_star = not this.italic_star
                    } else {
                        ~add_text()
                        this.bold_star = not this.bold_star
                    }
                } else {
                    ~add_text()
                    this.italic_star = not this.italic_star
                }
            } else if (this.allow_strikethrough && this.check_adv("~")) {
                if (this.check_adv("~")) {
                    ~og_text.append("~")
                    ~add_text()
                    this.strikethrough = not this.strikethrough
                } else {
                    ~elem.append("~")
                }
            } else if (this.check("[") || this.text[this.i..this.i+2] == "![") {
                ~add_text()
                toks += this.link()
            } else if (this.check_adv("\\")) {
                escape = true
            } else if (this.check_adv("\n")) {
                if (elem.length() > 0 && elem[-1] == " ")
                    ~elem.pop()
                break
            } else if (c in end_on) {
                break
            } else {
                if (escape)
                    ~elem.append("\\")
                ~elem.append(this.text[this.i])
                ~this.compact_spaces()
                this.i += 1
            }
        }
        ~add_text()
        if (toks.empty() && not og_text.empty()) {
            ~toks.append(MDText("".join(og_text)))
        }
        return toks
    }

    fun paragraph() {
        if (this.i < this.text.length()) {
            txt = this.textual()
            // Check if the paragraph could be hline
            if (txt.length() == 1 && txt[0].type == MDToken.TokenType.TEXT) {
                value = txt[0].value
                while (not value.empty() && value[-1] == "\n")
                    value = value[0..value.length()-1]
                while (not value.empty() && value[0] == "\n")
                    value = value[1..value.length()]
                if (value.length() > 2) {
                    if (all(value, fun(c)=c=="-") || all(value, fun(c)=c=="*") || all(value, fun(c)=c=="_")) {
                        return MDHLine(value)
                    }
                }
            } 
            return MDParagraph(txt)
        }
        return MDParagraph([])
    }

    // TODO: Support underlined heading (https://daringfireball.net/projects/markdown/basics)
    //     Title
    //     =====
    fun heading() {
        level = 0;
        while (this.check_adv("#")) {
            level += 1
        }
        // # without space after it is a normal text
        if (not this.check_adv(" ") && not this.check_adv("\t")) {
            this.i -= level
            return this.paragraph()
        }
        ~this.skip_ws();
        assert(level > 0, "Heading level is 0")
        txt = this.textual()
        // Remove possible # at the end
        last_text = txt[-1]
        for (i: last_text.value.length()-1..0) {
            if (last_text.value[i] != "#") {
                break
            }
            // TODO: Leave # if it was escaped
        }
        if (i < last_text.value.length()) {
            last_text.value = last_text.value[0..i+1]
        }
        return MDHeading(txt, level)
    }

    fun code_block(tabbed:Bool=false) {
        if (not tabbed) {
            assert(this.text[this.i..this.i+3] == "```", "Called code_block without ```")
            this.i += 3;
        }
        elem = []
        format = []
        format_parsed = false
        // Parse until new line or ``` to get the format
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            if (not tabbed && not format_parsed && this.check_adv("\n")) {
                format_parsed = true
            } else if (not tabbed && this.text[this.i..this.i+3] == "```") {
                this.i += 3
                // Skip possible new line
                ~this.check_adv("\n")
                break
            } else if (tabbed && this.check_adv("\n")) {
                space_am = 0
                while (this.check_adv(" ") && space_am < 4) {
                    space_am += 1
                }
                if (space_am == 4 || this.check_adv("\t")) {
                    ~elem.append("\n")
                } else {
                    break
                }
            } else {
                if (not tabbed && not format_parsed) {
                    ~format.append(c)
                } else {
                    ~elem.append(c)
                }
                this.i += 1
            }
        }
        return MDCodeBlock("".join(elem), "".join(format))
    }

    // Allow for codeblocks and headings in block quotes
    fun block_quotes() {
        level = 0;
        while (this.check_adv(">")) {
            level += 1
        }
        ~this.skip_ws()
        return MDBlockQuotes(this.textual(), level)
    }

    //@static_method
    fun get_ol_number(text, i) {
        val = ""
        ok = false
        for (j, c : Enumerate(text[i..text.length()], i)) {
            if (ord(c) >= 48 && ord(c) <= 57) {
                val ++= c
            } else if (c == ".") {
                if (j+1 >= text.length() || text[j+1] != " ")
                    return nil
                ok = true
                break
            } else {
                return nil
            }
        }
        if (not ok)
            return nil
        return Int(val)
    }

    fun listing(bullet, ordered:Bool) {
        result = MDList([], ordered, bullet)
        stack = [result]
        indent_level = [0]
        bullet_types = [bullet]

        while (this.i < this.text.length()) {
            indent = 0
            while (this.check_adv(" ")) {
                indent += 1
            }
            num = this.get_ol_number(this.text, this.i)
            if (num != nil)
                ordered = true
            else
                ordered = false
            if ((not ordered && this.text[this.i..this.i+2] in ["+ ", "- ", "* "]) || (ordered && num != nil)) {
                if (ordered) {
                    curr_bullet = num
                    this.i += String(num).length() + 2
                } else {
                    curr_bullet = this.text[this.i]
                    this.i += 2
                }
                level = indent

                while (level < indent_level[-1]) {
                    ~stack.pop()
                    ~indent_level.pop()
                    ~bullet_types.pop()
                }
                if (level > indent_level[-1]) {
                    new_list = MDList([], ordered, curr_bullet)
                    ~stack[-1].value.append(new_list)
                    ~stack.append(new_list)
                    ~indent_level.append(level)
                    ~bullet_types.append(curr_bullet)
                } else if (not ordered && curr_bullet != bullet_types[-1] && level == 0) {
                    this.i -= 2
                    break
                }
                ~stack[-1].value.append(this.paragraph())
            } else if (this.check_adv("\n"))
                break
            else {
                new_p = this.paragraph()
                new_p.value = [MDText(" ")] + new_p.value
                ~stack[-1].value[-1].value.append(new_p)
            }
        }

        return result
    }

    fun table_row() {
        // TODO: Add support for escaping pipes.
        line_start = this.i
        j = 0
        bar_count = 0
        // Find trimmed line start and end
        while (this.i + j < this.text.length() && this.text[this.i + j] != "\n") {
            c = this.text[this.i + j]
            if (c == " " || c == "\t") {
                if (bar_count <= 0) // trimming spaces
                    line_start += 1
            }
            else if (c == "|")
                bar_count += 1
            else if (bar_count <= 0) {
                // Not a table - we found something else on the start than | or ws.
                return nil
            }
            j += 1
        }
        if (bar_count <= 2) // There has to be at least 3 bars
            return nil
        line = this.text[line_start..this.i+j]
        cells = []
        // Start from 1 since we know on 0 is |
        start = 1
        for (a : 2..line.length()) {
            if (line[a] == "|") {
                cell_str = line[start..a].strip()
                cell_parser = MDParser(cell_str, this.allow_strikethrough)
                v = cell_parser.parse()
                if (v.empty())
                    ~v.append(MDParagraph([]))
                ~cells.append(v)
                start = a + 1
            }
        }
        // Check that start is over length or that all the symbols after it
        // are white spaces.
        if (start < line.length()) {
            if (not all(line[start..line.length()], fun(x)=x==" " || x == "\t"))
                return nil
        }
        this.i += j + 1
        return cells
    }

    fun table() {
        t = []
        do {
            row = this.table_row()
            if (row != nil)
                ~t.append(row)
        } while (row != nil)
        if (t.empty())
            return [false, nil]
        // Detect header
        has_header = false
        if (t.length() > 1) {
            possible_header = t[1]
            alignments = []
            has_header = true
            for (c: possible_header) {
                if (c.length() == 1) {
                    if (c[0].type == MDToken.TokenType.PARAGRAPH) {
                        text = c[0].value
                        if (text.length() != 1 || text[0].type != MDToken.TokenType.TEXT) {
                            has_header = false
                            break
                        }
                        v = text[0].value
                        if (v.length() < 4) {
                            has_header = false
                            break
                        }
                        if (v[0] == ":" && v[-1] == ":" && all(v[1..v.length()-1], fun(c)=c=="-")) {
                            ~alignments.append(MDTable.CELL_ALIGNMENT.CENTER)
                        } else if (v[0] == ":" && all(v[1..v.length()], fun(c)=c=="-")) {
                            ~alignments.append(MDTable.CELL_ALIGNMENT.LEFT)
                        } else if (v[-1] == ":" && all(v[0..v.length()-1], fun(c)=c=="-")) {
                            ~alignments.append(MDTable.CELL_ALIGNMENT.RIGHT)
                        } else {
                            has_header = false
                            break
                        }
                    } else if (c[0].type == MDToken.TokenType.HLINE) {
                        ~alignments.append(MDTable.CELL_ALIGNMENT.DEFAULT)
                    } else {
                        has_header = false
                        break
                    }
                } else {
                    has_header = false
                    break
                }
            }
        }
        if (has_header) {
            ~t.pop(1)
            return [true, MDTable(t, true, alignments)]
        }
        return [true, MDTable(t)]
    }

    fun parse() {
        tokens = []

        elem = []
        
        this.bold = false
        this.bold_star = false
        this.italic = false
        this.italic_star = false
        this.code = false
        this.strikethrough = false

        while (this.i < this.text.length()) {
            c = this.text[this.i]
            ol_num = this.get_ol_number(this.text, this.i)
            // Detect table only after new line
            parsed_table = false
            table = nil
            if (this.i > 0 && this.text[this.i-1] == "\n") {
                // TODO: Use just nil or table
                parsed_table, table = this.table()
            }
            if (parsed_table) {
                ~tokens.append(table)
            } else if (this.check("#")) {
                ~tokens.append(this.heading())
            } else if (this.check(">")) {
                ~tokens.append(this.block_quotes())
            } else if (this.text[this.i..this.i+3] == "```") { // TODO: Add code block indented with tab or 4 spaces
                ~tokens.append(this.code_block())
            } else if (this.check(" ") || this.check("\t")) {
                if (this.check_adv("\t")) {
                    ~tokens.append(this.code_block(true))
                } else {
                    space_am = 0
                    while (this.check_adv(" ") && space_am < 4) {
                        space_am += 1
                    }
                    // If at least 4 spaces, then code, otherwise just keep them
                    // removed.
                    if (space_am == 4) {
                        ~tokens.append(this.code_block(true))
                    }
                }
            } else if (this.check("\n")) {
                ~tokens.append(MDParagraph([]))
                while(this.check_adv("\n")) {
                    // skip new lines
                }
            } else if (this.text[this.i..this.i+2] in ["+ ", "- ", "* "]) {
                bullet = this.text[this.i]
                ~tokens.append(this.listing(bullet, false))
            } else if (ol_num != nil) {
                ~tokens.append(this.listing(ol_num, true))
            } else {
                p = this.paragraph()
                // Join multiple paragraphs into one
                if (tokens.length() > 0 && (tokens[-1].type == MDToken.TokenType.PARAGRAPH || tokens[-1].type == MDToken.TokenType.BLOCK_QUOTES) && tokens[-1].value != []) {
                    tokens[-1].value += [MDText(" ")] + p.value
                } else
                    ~tokens.append(p)
            }
        }

        return tokens
    }
}

fun parse_md(text:String, allow_strikethrough:Bool=true) {
    parser = MDParser(text, allow_strikethrough)
    return parser.parse()
}