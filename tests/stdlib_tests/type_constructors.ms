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
List([1,2,3])
"\n"
List([true, nil, 2, 4])
"\n"
List("Hello")
"\n"

class IterableClass {
    fun IterableClass(l) {
        this.l = l
    }

    fun __iter() {
        return this.l
    }
}

ic = IterableClass([5,6,7,8])
List(ic)
"\n"
List(ic)
"\n"
List(IterableClass(""))
"\n"

class NextClass {
    fun NextClass(max) {
        this.max = max
    }

    fun __iter() {
        this.i = 0
        return this;
    }

    fun __next() {
        if (this.i > this.max)
            raise StopIteration()
        v = this.i
        this.i += 1
        return v
    }
}

nc = NextClass(5)
List(nc)
"\n"
List(nc)
"\n"
List(NextClass(-8))
"\n"

e1 = Exception("Some exception")
e1 ++ "\n"

assert(Dict() == {:})
assert(Dict([["A", 1], ["B", 2], ["C", 3]]) == {"A": 1, "B": 2, "C": 3})
assert(Dict([]) == {:}) 
try {
    Dict([1,2,3])
} catch (e:TypeError) {
    "caught\n"
}

try {
    Dict(1)
} catch (e:TypeError) {
    "caught\n"
}

try {
    Dict([[1,2],[3,4,5]])
} catch (e:TypeError) {
    "caught\n"
}