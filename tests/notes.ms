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