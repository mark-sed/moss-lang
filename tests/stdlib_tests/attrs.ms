class MyValue {
    fun MyValue(x) {
        this.x = x
    }

    fun foo(a) {
        this.a = 5
    }

    fun __Int() {
        return this.x
    }

    fun __String() {
        return this.x 
    }

    fun __Float() {
        return Float(this.x)
    }

    fun __Bool() {
        return this.x > 0
    }
}

class TheirValue : MyValue {
    NAME = "tv"

    fun TheirValue() {}
}

class SomeClas {}

tv = TheirValue()
mv = MyValue(42)

// Anonymous lambda should not be listed and neither anonymous space
fun(a, b)=a*b
space {
    
}

attrs(mv)
"\n"
attrs(MyValue)
"\n"
attrs(tv)
"\n"
attrs(SomeClas())
"\n"
attrs()
"\n"

class SomeClassFoo {
    NAME = "scf"

    fun SomeClassFoo() {
        this.a = "a"
        this.b = 42
        this.name = NAME
    }

    @staticmethod
    fun foo() {}

}

scf = SomeClassFoo();
hasattr(scf, "NAME")
", "
hasattr(SomeClassFoo, "NAME")
", "
hasattr(scf, "someval")
", "
hasattr(scf, "name")
", "
hasattr(SomeClassFoo, "name")
", "
hasattr(scf, "SomeClassFoo")
", "
hasattr(scf, "a")
", "
hasattr(scf, "b")
"\n"

getattr(scf, "a")
", "
getattr(scf, "b")
", "
getattr(SomeClassFoo, "NAME")
", "
try
    getattr(SomeClassFoo, "name")
catch(a:AttributeError)
    "caught\n"

~setattr(scf, "new_attr", -7)
scf.new_attr
", "

fun new_fun() {
    return this.name
}
~setattr(scf, "new_fun", new_fun)
scf.new_fun()
", "
~setattr(SomeClassFoo, "new_fun", new_fun)
new_scf = SomeClassFoo()
new_scf.new_fun()
", "

some_int = 42
some_str = "hello"
try
    setattr(some_int, "a", 7)
catch(a:AttributeError)
    "caught\n"

~delattr(scf, "new_fun")
try
    scf.new_fun()
catch(a:AttributeError)
    "caught, "
~delattr(SomeClassFoo, "new_fun")
try
    SomeClassFoo().new_fun()
catch(a:AttributeError)
    "caught, "

try
    ~delattr(scf, "nonexistentval")
catch(a:AttributeError)
    "caught, "

try
    ~delattr(some_str, "capitalize")
catch(a:AttributeError)
    "caught\n"