a = 42
vardump(a)

// TODO: Add esc sequences
vardump("""
hello
\tthere""")
vardump("")

vardump(a > 4)

vardump(nil)

l = [1, "two", true, [8, 9]]
vardump(l)

vardump([])

enum MyEnum {
    A, B, C
}

vardump([[MyEnum]])

enum EmptyEnum {}

vardump(EmptyEnum)

class A {}
class B {}

class MyClass : A, B {

    NAME = "myclass"

    fun MyClass(n:[Int, Float]) {
        this.n = n 
    }

    fun get_n() {
        return this.n
    }
}

vardump(MyClass)

class FooCls {
    me = FooCls
    NAME = "FooCls"
}

f = FooCls()
f.i = 4
f.a = f
f.again = f
vardump(f)

space FooSpc {
    pi = 3.14
    me = FooSpc
    foo_class = FooCls
}

vardump(FooSpc)

dct = {"a": 1, "b": 2, "dct": nil, "l":[1,[0,0],2]}
dct["dct"] = dct
vardump(dct)

lst = [1, [0,0], 2]
~lst.append(lst)
vardump(lst)