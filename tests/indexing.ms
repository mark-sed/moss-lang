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