[[1], [3]]
"\n"
[[1,2], [3], 5, (1,2..5)]
"\n"
[1, 4..5] // 1, range
"\n"

fun add(a, b) {
    return a + b
}
assert(add(2, 3) == 5)

fun foo() = 1

[foo(), foo()]
"\n"
l = [1, "two", true, [8, 9], []]
l
"\n"

fun varargsf(a:[Int,Bool]=1, ... b) {
    return a + b[0]
}

varargsf(5, 1, 2)
"\n"

varargsf(6, 1, 2..5)
"\n"

fun varargsf2(... a) {
    return a
}

varargsf2(1, 3, 4 .. -1)
"\n"
varargsf2((6, 4 .. -1))
"\n"
varargsf2([1, 2..5])
"\n"
varargsf2([(1, 2..5)])
"\n"