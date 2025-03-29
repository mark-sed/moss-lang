// Strings
"a" in "hello"
"\n"
"ello" in "hello"
"\n"
a = "ello"
a in "hell"
"\n"
str = "abcdefg123"
try
    1 in str
catch(e:TypeError)
    "caught\n"

"1" in str
"\n"
a in str
"\n"

for(c:str) {
    assert(c in str, "value from string fails on 'in'")
}

// Lists
lst_in1 = [1, 2, true, nil, List]
1 in lst_in1
"\n"
"1" in lst_in1
"\n"
List in lst_in1
"\n"

for (v: lst_in1) {
    assert(v in lst_in1, "value from list fails on 'in'")
}

45 in []
"\n"

// Dicts
dct_in1 = {1: "1", "a": 1, true : Int, Int: Float, false: 0}
1 in dct_in1
"\n"
true in dct_in1
"\n"
false in dct_in1
"\n"
0 in dct_in1
"\n"
String in dct_in1
"\n"
Int in dct_in1
"\n"

0x2a in {:}
"\n"
"" in {"":""}
"\n"

