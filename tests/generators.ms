@converter("txt", "html")
fun txt2html(txt) {
    return "<p>" ++ txt ++ "</p>\n"
}

@converter("md", "html")
fun md2html(txt) {
    txt = String(txt)
    if (txt.length() < 2)
        return "<p>"++txt++"</p>\n"
    if (txt[0..2] == "__" && txt[txt.length()-2..txt.length()] == "__")
        return "<b>"++txt[2..txt.length()-2]++"</b>\n"
    else if (txt[0] == "#") {
        if (txt[1] == "#")
            return "<h2>" ++ txt[2..txt.length()] ++ "</h2>\n"
        else
            return "<h1>" ++ txt[1..txt.length()] ++ "</h1>\n"
    }
    return "<p>"++txt++"</p>\n"
}

@generator("html")
fun html_gen(txts) {
    html"""\
<!DOCTYPE html>
<html>
<body>
"""
    for (t: txts) {
        t
    }
    html"""
</body>
</html>
"""
}

md"# Moss generated html"
"Hello, World!"

md"""## Heading 2"""
"""\n"""
md"""__Not correct converter__"""
"\n"
md"But works!"