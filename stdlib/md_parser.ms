d"""
Module for parsing markdown for conversions to other formats.
"""

class MDToken {

    enum TokenType {
        TEXT
        HEADING
        PARAGRAPH
        BLOCK_QUOTES
        LIST
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

    // TODO: Add links (with title support)
    // TODO: Add images (with title)
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

    fun __String() {
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

class MDList : MDToken {
    // TODO: Handle task list
    fun MDList(values, ordered:Bool, bullet:[String,Int]) {
        ~super(values, MDToken.TokenType.LIST)
        this.ordered = ordered
        this.bullet = bullet
        assert(not ordered || type(bullet) == Int, "Ordered list but bullet is not a number (" ++ bullet ++ ")")
    }

    fun indented(level:Int=0) {
        t = ""
        for (v: this.value) {
            if (type(v) == ::MDList) {
                t ++= v.indented(level+1)
            } else {
                t ++= ("  " * level) ++ this.bullet ++ (this.ordered ? ". " : " ") ++ v ++ "\n"
                if (this.ordered)
                    this.bullet += 1
            }
        }
        return t
    }

    fun __String() {
        if (this.ordered)
            this.i = this.bullet
        return this.indented()
    }
}

class MDParser {

    fun MDParser(text:String, allow_strikethrough:Bool=true) {
        this.text = text
        this.i = 0
        this.allow_strikethrough = allow_strikethrough
        if (this.allow_strikethrough)
            this.ESCAPE_LIST = "`*_[]()#+-.!>\\~"
        else
            this.ESCAPE_LIST = "`*_[]()#+-.!>\\"
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
        while((this.i+1 < this.text.length()) && (check(" ") || check("\t")) && (this.text[this.i+1] == " " || this.text[this.i+1] == "\t")) {
            this.i += 1
        }
    }

    fun textual() {
        fun add_text() {
            if (elem.length() == 0)
                return
            ~$toks.append(MDText("".join($elem), italic=this.italic, bold=this.bold, code=this.code, strikethrough=this.strikethrough))
            $elem = []
        }
        elem = []
        toks = []
        escape = false
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            if (escape && c in this.ESCAPE_LIST) {
                ~elem.append(this.text[this.i])
                this.i += 1
                escape = false
            } else if (check_adv("`")) {
                ~add_text()
                this.code = not this.code
            } else if (not this.code && check_adv("_")) { // TODO: Handle also *, but has to match
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
            } else if (this.allow_strikethrough && check_adv("~")) {
                if (check_adv("~")) {
                    ~add_text()
                    this.strikethrough = not this.strikethrough
                } else {
                    ~elem.append("~")
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
        if (this.i < this.text.length()) {
            // TODO: Remove possible spaces from ends of paragraphs.
            return MDParagraph(textual())
        }
        return MDParagraph([])
    }

    // TODO: Support underlined heading (https://daringfireball.net/projects/markdown/basics)
    //     Title
    //     =====
    // TODO: Remove possible # from the end of heading (allowed)
    //     ### This is an H3 ######
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
        return MDHeading(textual(), level)
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
            if (not tabbed && not format_parsed && check_adv("\n")) {
                format_parsed = true
            } else if (not tabbed && this.text[this.i..this.i+3] == "```") {
                this.i += 3
                // Skip possible new line
                ~check_adv("\n")
                break
            } else if (tabbed && check_adv("\n")) {
                space_am = 0
                while (check_adv(" ") && space_am < 4) {
                    space_am += 1
                }
                if (space_am == 4 || check_adv("\t")) {
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
        while (check_adv(">")) {
            level += 1
        }
        while (check_adv(" ") || check_adv("\t")) {
            // Skip spaces
        }
        return MDBlockQuotes(textual(), level)
    }

    @staticmethod
    fun get_ol_number(text, i) {
        val = ""
        ok = true
        for (j, c : Enumerate(text[i..text.length()], i)) {
            if (ord(c) >= 48 && ord(c) <= 57) {
                val ++= c
            } else if (c == ".") {
                if (j+1 >= text.length() || text[j+1] != " ")
                    return nil
                break
            } else {
                return nil
            }
        }
        return Int(val)
    }

    fun listing(bullet, ordered:Bool) {
        result = MDList([], ordered, bullet)
        stack = [result]
        indent_level = [0]
        bullet_types = [bullet]

        while (this.i < this.text.length()) {
            indent = 0
            while (check_adv(" ")) {
                indent += 1
            }
            num = nil
            if (ordered)
                num = this.get_ol_number(this.text, this.i)
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
                ~stack[-1].value.append(paragraph())
            } else if (check_adv("\n"))
                break
            else {
                new_p = paragraph()
                new_p.value = [MDText(" ")] + new_p.value
                ~stack[-1].value[-1].value.append(new_p)
            }
        }

        return result
    }

    fun parse() {
        tokens = []

        elem = []
        
        this.bold = false
        this.italic = false
        this.code = false
        this.strikethrough = false

        while (this.i < this.text.length()) {
            c = this.text[this.i]
            ol_num = this.get_ol_number(this.text, this.i)
            if (check("#")) {
                ~tokens.append(heading())
            } else if (check(">")) {
                ~tokens.append(block_quotes())
            } else if (this.text[this.i..this.i+3] == "```") { // TODO: Add code block indented with tab or 4 spaces
                ~tokens.append(code_block())
            } else if (check(" ") || check("\t")) {
                if (check_adv("\t")) {
                    ~tokens.append(code_block(true))
                } else {
                    space_am = 0
                    while (check_adv(" ") && space_am < 4) {
                        space_am += 1
                    }
                    // If at least 4 spaces, then code, otherwise just keep them
                    // removed.
                    if (space_am == 4) {
                        ~tokens.append(code_block(true))
                    }
                }
            } else if (check("\n")) {
                ~tokens.append(MDParagraph([]))
                while(check_adv("\n")) {
                    // skip new lines
                }
            } else if (this.text[this.i..this.i+2] in ["+ ", "- ", "* "]) {
                bullet = this.text[this.i]
                ~tokens.append(listing(bullet, false))
            } else if (ol_num != nil) {
                ~tokens.append(listing(ol_num, true))
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

/*f = open(args[0], "r")
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
