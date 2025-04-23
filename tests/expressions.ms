#!/usr/bin/moss

1 + 42 - 8 * 2
"\n"
(99 / 3) % 20
"\n"
4 ^ 2 ^ 2 + 5
"\n"
4 ^ 0.5 == 2
"\n"
"hello there" != "Hello"++" there"
"\n"
"abc" > "bcd"
"\n"
99 < 33 * 2
"\n"
42 >= 0x2a
"\n"
8 <= 64
"\n"
true and true
"\n"
4 > 5 or 8 < 4
"\n"
1 or 8 and 11
"\n"
0XFF and 0XFF
"\n"
12 xor 12
"\n"
0x1a_2b ++ "\n"
-42
"\n"
a = 1
not (a < 42)
"\n"
"abcdefg"[0]
"abc"[2]
"abcdef"[-1]
"abcdef"[-4]
"\n"[0]

// Ternary if

true ? "hi\n" : "no"
false ? "no" : "there\n"

fun get_true() {
    return true ? true : false
}

ter_v = get_true() ? "true\n" : "false\n"
ter_v

try {
    [1,2,3] ? "1" : "2"
} catch (e:TypeError) {
    "caught\n"
}

list1 = [1,2,3]
[4,5,6] + list1
"\n"
list1 + [4]
"\n"
list1
"\n"
[1,2,3,4] + [5, 6]
"\n"
[1] + [2]
"\n"
[] + []
"\n"
[] + [2, 4]
"\n"

// String multiplication
"-" * 5
"" * 10
"hello there" * 0 // Empty
"hi all" * -10 // Empty
"\n"
mult = 3
st = ">+"
st * mult 
"\n"
"🐈" * mult
"\n"

// List multiplication
[1,2] * 0
"\n"
sto = 100
[] * sto
"\n"

[1,2,3] * 3
"\n"
lstml1 = [true, false]
lstml2 = lstml1 * 2
assert(lstml1 != lstml2)
lstml1
"\n"
lstml2
"\n"

lstml1 * -2
"\n"
lstml2 * (sto - 100)
"\n"

[[[]]] * 3
"\n"
dva = 2
[[1], [2, [3]]] * dva
"\n"
onelst = [{"a": dva}]
onelst * dva
"\n"