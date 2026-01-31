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
// TODO: Fix once dict order is set
//d = {"a": 1, "b": 2, "c": 3}
//" ".join(d)
e1 = {"a": 1}
" ".join(e1)
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
hex(-2)
"\n"

bin(12)
"\n"
bin(0)
"\n"
bin(1)
"\n"
bin(8)
"\n"
bin(-8)
"\n"

oct(0q10)
"\n"
oct(42)
"\n"
oct(-8)
"\n"

"hello world\n".replace("world", "moss")
"banana".replace("a", "o")
"\n"
"abcabcabc\n".replace("abc", "x", count=-1)

"aaa".replace("a", "b", 2)
"\n"
"111222111".replace("1", "x", 1) 
"\n"
"ababab".replace("ab", "cd", 2)
"\n"

"hello world".replace(" ", "")
"\n"
"abracadabra".replace("a", "")
"\n"

// TODO
//"abc".replace("", "-")
//"\n"
//"".replace("", "X")
//"\n"

"unchanged".replace("a", "a")
"\n"
"repeat\n".replace("e", "gg", 0)

"hello\n".replace("z", "x")
"".replace("a", "b")

"café\n".replace("é", "e")
"Hello".replace("h", "y")
"\n"

"vAv".replace("v", "✅")
"\n"
"✅ Value\n".replace("✅", "❌")

type("abc".__iter())
"\n"

"""ABCDEFGHIAabC
""".multi_replace([["A", "a"], ["B", "b"]])
try
    "Hi".multi_replace([[]])
catch(e:ValueError)
    "caught\n"

try
    "Hi".multi_replace([["a", "b", "c"]])
catch(e:ValueError)
    "caught\n"

try
    "Hi".multi_replace([["a", "b"], 1])
catch(e:ValueError)
    "caught\n"

"Hi\n".multi_replace([])

"AaAcBBbAcaaaBaB\n".multi_replace([["b", "B"], ["B", "a"], ["a", "A"]])

str = "hello\nthere\nfrom\nmoss"
str.split()
"\n"
str.split(maxsplit=2)
"\n"
str.split(maxsplit=0)
"\n"

str2 = "1--2--3-4--5--"
str2.split("--")
"\n"
str2.split("-")
"\n"
str2.split("--", maxsplit=1)
"\n"
"aabccbeeb".split("b")
"\n"

"aaa".count("aa")
"\n"
"abcabdda".count("a")
"\n"
"".count("a")
"\n"
"".count("")
"\n"
"abcd".count("")
"\n"
"hello".count("hello!")
"\n"
md"""
Hello
There
Mr. Hello!
""".count("Hello")
"\n"

"abcdef".index("ab")
"\n"
"".index("bh")
"\n"
"hello, there\n".index("there")
"\n"
"hello".index("hello!")
"\n"
"".index("")
"\n"
"".index("a")
"\n"

txt = """
hello
there
sir
"""
txt[txt.index("sir")..txt.length()]

txt.index("sir", 0, 14)
"\n"

"abcde".index("e", 4, 5)
"\n"

"".index("jh", 100, 101)

"\n"
"loool".index("l", start=1, end=100)
"\n"

"aAbBcCd".swapcase()
"\n"
" a - 2 č d α".swapcase()
"\n"
"".swapcase()
"hHHel23-o".swapcase()
"\n"

"čřž\n".upper()
"7ÝŽČ\n".lower()

"žába!".capitalize()
"\n"

// Basic matches
"hello".rindex("l")        // 3
"\n"
"hello".rindex("lo")       // 3
"\n"
"banana".rindex("ana")     // 3
"\n"
"abracadabra".rindex("a")  // 10
"\n"

// When substring appears only once
"python".rindex("py")      // 0
"\n"
"python".rindex("on")      // 4
"\n"

// With start and end argument
"hello world".rindex("o", 0, 5)     // 4
"\n"
"hello world".rindex("o", 5, 11)    // 7
"\n"
"mississippi".rindex("ss", 0, 5)    // 2
"\n"
"mississippi".rindex("ss", 3, 11)   // 5
"\n"

// Whole-range explicit indices
"abcabcabc".rindex("abc", 0, 9)     // 6
"\n"
"abcabcabc".rindex("abc", 0, 6)     // 3
"\n"

// Single-character searches
"test".rindex("t")         // 3
"\n"
"test".rindex("e")         // 1
"\n"
"test".rindex("s")         // 2
"\n"

// Searching empty string
"hello".rindex("")         // 5  (empty string is found at len(s))
"\n"
"abc".rindex("", 0, 2)     // 2
"\n"
"abc".rindex("", 0, 3)     // 3
"\n"

// Edge cases
"aaaa".rindex("aa")        // 2
"\n"
"aaaa".rindex("aaa")       // 1
"\n"

// Errors (-1)
"hello".rindex("z")        // -1
"\n"
"hello".rindex("l", 0, 2)  // -1 (no 'l' in 'he')
"\n"
"abc".rindex("abcd")       // -1 (substring longer than string)
"\n"