v = Int(56)
v2 = Int("56")
v3 = Int("0x2a", 16)
v4 = Int("1010", 2)
v5 = Int(0.5)
v6 = Int(22.4)
v7 = Int(100)
v8 = Int()
v9 = Int(base=10, v="-8")

v++"\n"++v2++"\n"++v3++"\n"++v4++"\n"
v5++"\n"++v6++"\n"++v7++"\n"++v8++"\n"++v9++"\n"

e1 = Exception("Some exception")
e1 ++ "\n"