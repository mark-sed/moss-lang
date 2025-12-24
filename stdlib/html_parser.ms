d"""
Module for parsing html for conversions to other formats.
"""

class HTMLToken {

    enum TokenType {
        TEXT
        LINK
        IMAGE
        HEADING
        PARAGRAPH
        BLOCK_QUOTES
        LIST
        CODE_BLOCK
        HORIZONTAL_RULE
        TABLE
    }

    fun HTMLToken(value, type:TokenType) {
        this.value = value
        this.type = type
    }

    fun __String() {
        return String(this.value)
    }
}

class HTMLText : HTMLToken {

    replace_mappings = [
        ["&", "&amp;"],
        ["<", "&lt;"],
        [">", "&gt;"],
        ["\"", "&quot;"],
        ["'", "&#39"],
        ["\n", "<br>"]
    ]

    fun HTMLText(text:String, 
                 italic:Bool=false,
                 bold:Bool=false,
                 code:Bool=false,
                 strikethrough:Bool=false) {
        ~super(text.multi_replace(replace_mappings), HTMLToken.TokenType.TEXT)
        this.italic = italic
        this.bold = bold
        this.code = code
        this.strikethrough = strikethrough
    }

    fun __String() {
        text = this.value
        if (this.code)
            text = "<code>" ++ text ++ "</code>"
        if (this.italic)
            text = "<i>" ++ text ++ "</i>"
        if (this.bold)
            text = "<b>" ++ text ++ "</b>"
        if (this.strikethrough)
            text = "<del>" ++ text ++ "</del>"
        return text
    }
}

class HTMLLink : HTMLToken {
    fun HTMLLink(text, url:String, title:String) {
        ~super(url, HTMLToken.TokenType.LINK)
        this.text = text
        this.url = url
        this.title = title
    }

    fun __String() {
        return "<a href=\"" ++ this.url ++ "\" title=\"" ++ this.title ++ "\">" ++ ("".join([String(x) : x = this.text])) ++ "</a>"
    }
}

class HTMLImage : HTMLToken {
    fun HTMLImage(text, url:String, title:String) {
        ~super(url, HTMLToken.TokenType.IMAGE)
        this.text = text
        this.url = url
        this.title = title
    }

    fun __String() {
        return "<img src=\"" ++ this.url ++ "\" title=\"" ++ this.title ++ "\" alt=\"" ++ ("".join([String(x) : x = this.text])) ++ "\">"
    }
}

class HTMLHeading : HTMLToken {

    fun HTMLHeading(value, level:Int) {
        ~super(value, HTMLToken.TokenType.HEADING)
        this.level = level
    }

    fun __String() {
        return "<h" ++ this.level ++ ">" ++ ("".join([String(x) : x = this.value])) ++ "</h" ++ this.level ++ ">"
    }
}

class HTMLParagraph : HTMLToken {
    fun HTMLParagraph(value) {
        ~super(value, HTMLToken.TokenType.PARAGRAPH)
    }

    fun __String() {
        return "<p>"++"".join([String(x) : x = this.value])++"</p>"
    }
}

class HTMLCodeBlock : HTMLToken {
    fun HTMLCodeBlock(value, format:String) {
        ~super(value, HTMLToken.TokenType.CODE_BLOCK)
        this.format = format
    }

    fun __String() {
        return "<pre><code" ++ (this.format.length() > 0 ? " class=\"language-"++this.format++"\"" : "") ++ ">" ++ this.value ++ "</code></pre>"
    }
}

class HTMLBlockQuotes : HTMLToken {
    fun HTMLBlockQuotes(value, level) {
        ~super(value, HTMLToken.TokenType.BLOCK_QUOTES)
        this.level = level
    }

    fun __String() {
        return "<blockquote>" ++ ("".join([String(x) : x = this.value])) ++ "</blockquote>"
    }
}

class HTMLList : HTMLToken {
    fun HTMLList(values, ordered:Bool, start:Int=1) {
        ~super(values, HTMLToken.TokenType.LIST)
        this.ordered = ordered
        this.start = start
    }

    fun indented(start=1, level:Int=0) {
        t = ""
        for (v: this.value) {
            if (type(v) == ::HTMLList) {
                if (v.ordered)
                    t ++= "<ol start=\"" ++ v.start ++ "\">" ++ v.indented(v.start, level+1) ++ "</ol>"
                else
                    t ++= "<ul>" ++ v.indented(level=level+1) ++ "</ul>"
            } else {
                t ++= "<li>" ++ v ++ "</li>"
            }
        }
        return t
    }

    fun __String() {
        if (this.ordered)
            return "<ol start=\"" ++ this.start ++ "\">" ++ this.indented(this.start) ++ "</ol>"
        return "<ul>" ++ this.indented() ++ "</ul>"
    }
}

class HTMLHorizontalRule : HTMLToken {
    fun HTMLHorizontalRule() {
        ~super("", HTMLToken.TokenType.LINK)
    }

    fun __String() {
        return "<hr>"
    }
}

class HTMLTable : HTMLToken {
    enum CELL_ALIGNMENT {
        DEFAULT
        LEFT
        CENTER
        RIGHT
    }
    
    fun HTMLTable(value, has_header:Bool=true, alignments:[List,NilType]=nil, caption:[String,NilType]=nil) {
        ~super(value, HTMLToken.TokenType.TABLE)
        this.has_header = has_header
        this.alignments = alignments
        this.caption = caption
    }

    fun __String() {
        t = "<table>\n"
        if (this.caption != nil) {
            t ++= f"<caption>{this.caption}</caption>\n"
        }
        header_row = this.has_header
        for (row: this.value) {
            t ++= "<tr>\n"
            for (c, cell: Enumerate(row)) {
                for (v: cell) {
                    align = ""
                    if (this.alignments != nil && c < this.alignments.length() && this.alignments[c] != CELL_ALIGNMENT.DEFAULT) {
                        switch(this.alignments[c]) {
                            case CELL_ALIGNMENT.CENTER: align = " style=\"text-align: center;\""
                            case CELL_ALIGNMENT.LEFT: align = " style=\"text-align: left;\""
                            case CELL_ALIGNMENT.RIGHT: align = " style=\"text-align: right;\""
                        }
                    }
                    if (header_row)
                        t ++= f"<th{align}>{v}</th>\n"
                    else
                        t ++= f"<td{align}>{v}</td>\n"
                }
            }
            header_row = false
            t ++= "</tr>\n"
        }
        t ++= "</table>"
        return t
    }
}