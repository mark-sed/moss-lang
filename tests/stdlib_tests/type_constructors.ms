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

f1 = Float()
f2 = Float("2.5")
f3 = Float(50)
f4 = Float(0xF)

f1 ++ "\n" ++ f2 ++ "\n" ++ f3 ++"\n" ++ f4 ++ "\n"

Bool()
"\n"
Bool(true)
"\n"
Bool([])
"\n"
Bool([[1,2]])
"\n"
Bool(nil)
"\n"
Bool(NilType)
"\n"
Bool("")
"\n"
Bool("hello")
"\n"
Bool(0.0)
"\n"
Bool(print)
"\n"
Bool(0)
"\n"
Bool(-0.56)
"\n"
Bool(-0.0)
"\n"
Bool(86)
"\n"
Bool({:})
"\n"
Bool({"a": {:}})
"\n"

String()
String("Hi\n")
String(55)
"\n"
String(String)
"\n"
String(nil)
"\n"

NilType()
"\n"

List()
"\n"
List(1)
"\n"
List(true, nil, 2, 4)
"\n"
l1 = List(1,2)
List(l1, 2, l1[1..-1])
"\n"

e1 = Exception("Some exception")
e1 ++ "\n"