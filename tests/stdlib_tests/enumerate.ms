for (a, b: Enumerate([1,2,3])) {
    a ++ ". " ++ b ++ "\n"
}
for (i: Enumerate([1,2,3])) {
    i
}
"\n"
for (i: Enumerate([9, 8, 7, 6, 5], start=100)) {
    i
}
"\n"

// test iter creating new iterator
e = Enumerate(["a","b","c"])
for (c1, k1: e) {
    for (c2, k2: e) {
        c1++k1++c2++k2++"|"
    }
}
"\n"