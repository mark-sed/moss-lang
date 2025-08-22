class MyDict : Dict {
    fun __iter() = super.__iter()
    fun __String() = "[D]:"++this.__value
}

md = MyDict([["a", 1], ["b", 2], ["c", 3]])
type(md)
"\n"
md
"\n"
MyDict()
"\n"

md.pop("a")
"\n"
md
"\n"
md.length()
"\n"
md.empty()
"\n"

md.pop("fsfs", "nope\n")

~md.remove("c")
md
"\n"

~md.clear()
md
"\n"