d"""
Internal module to provide unified tooling for parsers in Moss.
"""

class TextualParser {

    fun TextualParser(text:String) {
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

    fun check_adv_str(v:String) {
        if (this.i + v.length() >= this.text.length())
            return false
        if (this.text[this.i..this.i+v.length()] == v) {
            this.i += v.length()
            return true
        }
        return false
    }

    fun skip_ws() {
        while (this.check_adv(" ") || this.check_adv("\t")) {
            // Skip spaces
        }
    }

    fun advance() {
        assert(this.i < this.text.length(), "Advancing beyond text length")
        c = this.text[this.i]
        this.i += 1
        return c
    }

}