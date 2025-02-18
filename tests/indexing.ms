a = "some text abc\n"
a[0]
a[-1]

"a"[0]
"b"[-1]
a[-1]


try {
    a[-100]
} catch (e:IndexError) {
    "oob\n"
}

try {
    a[100]
} catch (e:IndexError) {
    "oob\n"
}

[1, 2, 3][1]
[1, 2, 3][-1]

lst = [1, true, nil, "abc\n"]
lst[1]
lst[-1][-1]
lst[2]

"\n"
try {
    lst[10]
} catch (e:IndexError) {
    "oob\n"
}

// Range indexing

"abcdef"[1..4]
"\n"
"abcdef"[4..1]
"\n"
"abcdef"[-1..-4]
"\n"
"abcdef"[-3..3]
"\n"
"abcdef"[3..-3]
"\n"

"abcdef"[15..-1]
"\n"
"abv"[0..5]
"\n"
"abv"[2..-5]
"\n"
"abcdef"[0,10..10]
"\n"
"abcdef"[10..13]
"\n"
""[-1..2]
"\n"

[1,2,3,4,5,6][0..5]
"\n"
[1,2,3,4,5,6][5..0]
"\n"
[1,2,3,4,5,6][0,2..10]
"\n"
[1,2,3,4,5,6][5,3..-1]
"\n"
[][-10..5]
"\n"
[1,2,3][-5..0]
"\n"
[1,2,3,4,5,6][-6..0]
"\n"
[1,2,3,4,5,6][-1..-7]
"\n"