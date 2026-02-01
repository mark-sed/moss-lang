import inspect

fun foo(a:Int) {
    return "int a\n"
}

fun foo(a:String) {
    return "string a\n"
}

fun foo(a) {
    return "gen a\n"
}

foos = foo
for (f: foos) {
    for (f2: foos) {
        inspect.signature(f)++"+"
        inspect.signature(f2)++"|"
    }
}
"\n"

type([1,2,3].__iter())
"\n"
type([1,2,3].__iter().__iter())
"\n"
type(Bytes("").__iter())
"\n"
type(Bytes("123").__iter())
"\n"
type({:}.__iter())
"\n"
type({"a":2}.__iter().__iter())
"\n"
type("moss".__iter())
"\n"
type("lang".__iter().__iter())
"\n"

b = Bytes("a")
bi = BytesIterator(b)
type(bi)
"\n"
bi.__next()
"\n"

l = [0]
li = ListIterator(l)
type(li)
"\n["
li.__next()
try
    li.__next()
catch (e: StopIteration)
    "]\n"

s = "a"
si = StringIterator(s)
type(si) ++ "\n"
si.__next()
"\n"

d = {"a":1}
di = DictIterator(d)
type(di)
"\n"
di.__next()
"\n"

f = foo
fi = FunctionListIterator(f)
type(fi)
"\n"
inspect.signature(fi.__next())
"\n"