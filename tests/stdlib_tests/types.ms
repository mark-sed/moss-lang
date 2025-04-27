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