// Hash
hash(42)
"\n"
c = Int
hash(Int) == hash(c)
"\n"
hash("123") == hash("1"++2++3)
"\n" 
try {
    hash([1,2,3])
} catch (e:TypeError) {
    "caught\n"
}
try {
    hash({:})
} catch (e:TypeError) {
    "caught\n"
}

// Type function
type(42)
"\n"
type(67) == type(89)
"\n"
type(type) == type(Math.sin)
"\n"
type(moss.Math)
"\n"
type(Int)
"\n"
type(md"#H1")
"\n"
type(Type)
"\n"

// id
a = [1,2,3]
b = a
id(a) == id(b)
"\n"

id(true) == id(true)
"\n"

a = 5
b = a
id(a) == id(b)
"\n"

id(print) == id(print)
"\n"
f = print
id(f) == id(print)

"\n"
id("hello") == id("hello")
"\n"

id(5678) == id(5678)
"\n"

fun f1(a, b:Bool) {
    return "f11\n"
}

aa = f1

fun f1(a) {
    return "f1\n"
}

id(aa) == id(f1)
"\n"
id(f1) == id(f1)
"\n"