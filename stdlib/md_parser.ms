d"""
Module for parsing markdown for conversions to other formats.
"""

class MDToken {

    enum TokenType {
        TEXT
        HEADING
        PARAGRAPH
        BLOCK_QUOTES
        U_LIST
        O_LIST
        TASK_LIST
        CODE_BLOCK
        HLINE
        TABLE
    }

    fun MDToken(value, type:TokenType) {
        this.value = value
        this.type = type
    }

    fun __String() {
        return String(this.value)
    }
}

class MDText : MDToken {

    // TODO: Add strikethrough (if enabled)
    // TODO: Add links (with title support)
    // TODO: Add images (with title)
    // TODO: Autolink
    fun MDText(text:String, italic:Bool=false, bold:Bool=false, code:Bool=false) { 
        ~super(text, MDToken.TokenType.TEXT)
        this.italic = italic
        this.bold = bold
        this.code = code
    }

    fun __String() {
        text = this.value
        if (this.italic)
            text = "_" ++ text ++ "_"
        if (this.bold)
            text = "**" ++ text ++ "**"
        if (this.code)
            text = "`" ++ text ++ "`"
        return text
    }
}

class MDHeading : MDToken {

    fun MDHeading(value, level:Int) {
        ~super(value, MDToken.TokenType.HEADING)
        this.level = level
    }

    fun __String() {
        return ("#" * this.level) ++ ("".join([String(x) : x = this.value]))
    }
}

class MDParagraph : MDToken {
    fun MDParagraph(value) {
        ~super(value, MDToken.TokenType.PARAGRAPH)
    }

    fun __String() {
        t = []
        for (e: this.value) {
            ~t.append(String(e))
        }
        return "".join(t)
    }
}

class MDCodeBlock : MDToken {
    fun MDCodeBlock(value, format:String) {
        ~super(value, MDToken.TokenType.CODE_BLOCK)
        this.format = format
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

    fun __String() {
        return (">"*this.level) ++ ("".join([String(x) : x = this.value]))
    }
}

class MDParser {

    fun MDParser(text:String) {
        this.text = text
        this.i = 0
    }

    fun check_adv(v:String) {
        if (this.i >= this.text.length())
            return false
        if (this.text[this.i] == v) {
            this.i += 1
            return true
        }
        return false
    }

    fun check(v:String) {
        if (this.i >= this.text.length())
            return false
        if (this.text[this.i] == v) {
            return true
        }
        return false
    }

    fun compact_spaces() {
        while((check(" ") || check("\t")) && (this.text[this.i+1] == " " || this.text[this.i+1] == "\t")) {
            this.i += 1
        }
    }

    fun text() {
        fun add_text() {
            if (elem.length() == 0)
                return
            ~$toks.append(MDText("".join($elem), italic=this.italic, bold=this.bold, code=this.code))
            $elem = []
        }
        elem = []
        toks = []
        escape = false
        while (this.i < this.text.length()) {
            if (escape && this.text[this.i] in "`*_[]()#+-.!>\\") {
                ~elem.append(this.text[this.i])
                this.i += 1
                escape = false
            } else if (check_adv("`")) {
                ~add_text()
                this.code = not this.code
            } else if (check_adv("_")) { // TODO: Handle also *, but has to match
                if (check_adv("_")) {
                    if (check_adv("_")) {
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
            } else if (check_adv("\\")) {
                escape = true
            } else if (check_adv("\n")) {
                break
            } else {
                if (escape)
                    ~elem.append("\\")
                ~elem.append(this.text[this.i])
                ~compact_spaces()
                this.i += 1
            }
        }
        ~add_text()
        return toks
    }

    fun paragraph() {
        while (this.i < this.text.length()) {
            return MDParagraph(text())
        }
        return MDParagraph([])
    }

    fun heading() {
        level = 0;
        while (check_adv("#")) {
            level += 1
        }
        // # without space after it is a normal text
        if (not check(" ") && not check("\t")) {
            this.i -= level
            return paragraph()
        }
        assert(level > 0, "Heading level is 0")
        return MDHeading(text(), level)
    }

    fun code_block() {
        assert(this.text[this.i..this.i+3] == "```", "Called code_block without ```")
        this.i += 3;
        elem = []
        format = []
        format_parsed = false
        // Parse until new line or ``` to get the format
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            if (not format_parsed && check_adv("\n")) {
                format_parsed = true
            } else if (this.text[this.i..this.i+3] == "```") {
                this.i += 3
                // Skip possible new line
                ~check_adv("\n")
                break
            } else {
                if (not format_parsed) {
                    ~format.append(c)
                } else {
                    ~elem.append(c)
                }
                this.i += 1
            }
        }
        return MDCodeBlock("".join(elem), "".join(format))
    }

    fun block_quotes() {
        level = 0;
        while (check_adv(">")) {
            level += 1
        }
        while (check_adv(" ") || check_adv("\t")) {
            // Skip spaces
        }
        return MDBlockQuotes(text(), level)
    }

    fun parse() {
        tokens = []

        elem = []
        
        this.bold = false
        this.italic = false
        this.code = false

        while (this.i < this.text.length()) {
            c = this.text[this.i]
            if (check("#")) {
                ~tokens.append(heading())
            } else if (check(">")) {
                ~tokens.append(block_quotes())
            } else if (this.text[this.i..this.i+3] == "```") {
                ~tokens.append(code_block())
            } else if (check("\n")) {
                ~tokens.append(MDParagraph([]))
                while(check_adv("\n")) {
                    // skip new lines
                }
            } else {
                p = paragraph()
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

/*
f = open(args[0], "r")
lines = f.readlines()

parser = MDParser("\n".join(lines))
parsed = parser.parse()

for (p: parsed) {
    p
    "\n"
}

for (p: parsed) {
    p.type ++": " ++ p.value
    "\n"
}
*/
