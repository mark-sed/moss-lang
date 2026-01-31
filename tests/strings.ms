r"\n\tsdf\0x12\n"
"\n"
r"""|
\n\treturn
\b\t\n
"""

// Escape sequences

"\x2A\n"
"""\x41\X42\x43DEF
"""

"\Q052"
"""
\q101\q102\q103DEF
"""

"\Q062" == "\x32"
"\n"
String(2) == "\X32"
"\n"

"\u03A9"
"""
\U000003A9
"""
"> \U0001F409\u4E2D\u00F1\n"

s = "abcd"
for (a: s) {
    for (b: s) {
        a++b++"|"
    }
}
"\n"