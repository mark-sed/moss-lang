import math

try
    math.foo
catch (e:AttributeError)
    e

try
    Exception("").hih
catch (e:AttributeError)
    e

enum Foo {
    F
    G
}

try
    Foo.j
catch (e:AttributeError)
    e

try
    Foo.F.j
catch (e:AttributeError)
    e

try
    print.hi
catch (e:AttributeError)
    e

try
    [].hi
catch (e:AttributeError)
    e

try
    "".uu
catch (e:AttributeError)
    e

try
    (1).txt
catch (e:AttributeError)
    e

try
    (7.5).h
catch (e:AttributeError)
    e

try
    true.hi
catch (e:AttributeError)
    e

try
    nil.k
catch (e:AttributeError)
    e

try
    math.sin.hi
catch (e:AttributeError)
    e

try
    Complex(0).hi
catch (e:AttributeError)
    e

try
    (1..3).hi
catch (e:AttributeError)
    e

try {
    {:}.hi
} catch (e:AttributeError)
    e

class Foo : Exception {
    fun Foo() {
        try
            super.hi
        catch (e:AttributeError)
            e
    }
}

~Foo()

try
    ListIterator([1,2]).hi
catch (e:AttributeError)
    e

try
    DictIterator({:}).hi
catch (e:AttributeError)
    e

try
    StringIterator("").hi
catch (e:AttributeError)
    e

try
    BytesIterator(Bytes("")).hi
catch (e:AttributeError)
    e

try
    FunctionListIterator(math.sin).hi
catch (e:AttributeError)
    e

try
    Exception.hi
catch (e:AttributeError)
    e

space Foo {}

try
    Foo.hi
catch (e:AttributeError)
    e

try
    md"dg".hi
catch (e:AttributeError)
    e
