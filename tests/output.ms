1 + 40 + 1; ", "
true
", "; false
"\n"
"moss"++" "++"language\n"
0b1101
"\n"
0Q11
"\n"
0x2A
"\n"

// Recursive output
dct = {"a": 1, "b": 2, "dct": nil, "l":[1,[0,0],2]}
dct["dct"] = dct
dct
"\n"

lst = [1, [0,0], 2]
~lst.append(lst)
lst
"\n"

~print({"a": lst})