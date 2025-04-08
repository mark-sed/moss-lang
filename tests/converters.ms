@formatter
fun md4(x) = x

@converter("md4", "txt")
fun md4_to_txt(n) {
    txt = ""
    for (c: n) {
        if (c != "#")
            txt ++= c
    }
    return txt
}

md4"""#Title1
Paragraph1

#Title2
Paragraph2
"""