fun not_annotated(x) {
    return x ++ "formatted!\n"
}

not_annotated"Text"
"\n"

@formatter
fun md(x) {
    return x ++ "formatted!\n"
}

md"""# Title
Some text.
"""

Note()
Note("Some text\n")
mdn = Note("# Title1\n", "md")
mdn.format
"\n"
mdn.value

// Check that returned value is NoteValue
@formatter
fun md4(x) = x

@formatter
fun md5(x) = "conststr"

@formatter
fun md6(x) = Note(x, "txt")

a = md4"hello"
vardump(a)
vardump(md5(45))
vardump(md6("hello"))