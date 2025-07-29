d"""
Module for parsing markdown for conversions to other formats.
"""

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
        TABLE
    }

    fun MDToken(value, type:TokenType) {
        this.value = value
        this.type = type
    }

    fun to_txt() {
        return this.value
    }

    fun to_html() {
        assert(false, "Called to_html to pure MDToken")
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

    fun skip_ws() {
        while (check_adv(" ") || check_adv("\t")) {
            // Skip spaces
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
        image = check_adv("!")
        assert(this.text[this.i] == "[", "Called link parsing on non-link text -- does not start with [")
        this.i += 1
        text = textual("]")
        if (not check_adv("]")) {
            return cancel(image, text)
        }
        if (not check_adv("(")) {
            return cancel(image, text+[MDText("]")])
        }
        url = []
        while (this.i < this.text.length() && not this.text[this.i] in "\")\n") {
            ~url.append(this.text[this.i])
            this.i += 1
        }
        if (check("\n")) {
            return cancel(image, text, url)
        }
        if (check_adv(")")) {
            return [MDLink(text, "".join(url), "", image)]
        }
        title = []
        if (check_adv("\"")) {
            while (this.i < this.text.length() && not check("\"") && not check("\n")) {
                if (check_adv("\\")) {
                    // Possible escape of "
                    if (check_adv("\"")) {
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
        if (not check_adv("\"")) {
            return cancel(image, text, url, title)
        }
        ~skip_ws()
        if (check_adv(")")) // Last enclosing of link
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
        while (this.i < this.text.length()) {
            c = this.text[this.i]
            if (escape && c in this.ESCAPE_LIST) {
                ~elem.append(this.text[this.i])
                this.i += 1
                escape = false
            } else if (check_adv("`")) {
                ~add_text()
                this.code = not this.code
            } else if (not this.code && check_adv("_")) {
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
            } else if (not this.code && check_adv("*")) {
                if (check_adv("*")) {
                    if (check_adv("*")) {
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
            } else if (this.allow_strikethrough && check_adv("~")) {
                if (check_adv("~")) {
                    ~add_text()
                    this.strikethrough = not this.strikethrough
                } else {
                    ~elem.append("~")
                }
            } else if (check("[") || this.text[this.i..this.i+2] == "![") {
                ~add_text()
                toks += link()
            } else if (check_adv("\\")) {
                escape = true
            } else if (check_adv("\n")) {
                if (elem.length() > 0 && elem[-1] == " ")
                    ~elem.pop()
                break
            } else if (c in end_on) {
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
            return MDParagraph(textual())
        }
        return MDParagraph([])
    }

    // TODO: Support underlined heading (https://daringfireball.net/projects/markdown/basics)
    //     Title
    //     =====
    fun heading() {
        level = 0;
        while (check_adv("#")) {
            level += 1
        }
        // # without space after it is a normal text
        if (not check_adv(" ") && not check_adv("\t")) {
            this.i -= level
            return paragraph()
        }
        ~skip_ws();
        assert(level > 0, "Heading level is 0")
        txt = textual()
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
        ~skip_ws()
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
        this.bold_star = false
        this.italic = false
        this.italic_star = false
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

fun parse_md(text:String, allow_strikethrough:Bool=true) {
    parser = MDParser(text, allow_strikethrough)
    return parser.parse()
}