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