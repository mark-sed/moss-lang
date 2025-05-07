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

// chr
chr(42)
"\n"
try
    chr(1114112)
catch (e:ValueError)
    "caught\n"

chr(1114111) != "0"
"\n"
chr(0) != "0"
"\n"
try
    chr(-6)
catch (e:ValueError)
    "caught\n"

chr(109) ++ chr(111) ++ chr(115) ++ chr(115) ++ chr(10)

// ord
ord("*")
"\n"
try
    ord("moss")
catch(e:ValueError)
    "caught\n"
ord("m")
"\n"

// hex
hex(0)
"\n"
hex(42)
"\n"
hex(12648430)
"\n"