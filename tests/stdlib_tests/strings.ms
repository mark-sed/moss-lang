"123".length()
"\n"

a = "Hello, sir\t\n"
a.length()
"\n"

String.length("""
some vals
""")
"\n"

"abcd".reverse()
"\n"
b = "12345"
b.reverse() == b[-1..-b.length()-1]
"\n"

"\t\n  \t".strip()
st = "a \tb\t"
st.strip()
"\n"

"""HeLLo
THere madaM!
""".lower()

"""HeLLo! bACk to YOu - 98&*\n""".upper()

emstring = "x1234y5--🧆\n".lower()
emstring
emstring.upper().upper().lower().upper().capitalize()

str1 = "marek\n"
str1.capitalize()
"---hi\n".capitalize()

class ClsIt {
    fun ClsIt(x) {
        this.x = x
    }

    fun __iter() = this.x
}

", ".join([1,2,3,4,5])
"\n"
d = {"a": 1, "b": 2, "c": 3}
" ".join(d)
"\n"
"-".join(ClsIt([6,7,8,9]))
"\n"
" ".join("hi!")
"\n"