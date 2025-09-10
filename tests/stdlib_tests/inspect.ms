fun foo(a:Int, b:Bool, c=nil) {}

Inspect.signature(foo)
"\n"

fun foo2(a="hello", ... other) {
}

Inspect.signature(foo2)
"\n"

fun foo(a) {}

try
    Inspect.signature(foo)
catch (e:TypeError)
    "caught\n"

class Animal {
    fun Animal(name:String, age:[Int,Float]=0) {
        this.name = name;
        this.age = age;
    }
}

Inspect.signature(Animal.Animal)
"\n"

Inspect.signature(fun(a, b=1, c:Bool=false)=a)
"\n"

Inspect.signature(fun lambda_with_name()=4)
"\n"

enum En1 {A;}
class Cl2 {}
space Spc {}

Inspect.name(En1)
a = Cl2
"\n"
Inspect.name(a)
"\n"
Inspect.name(Spc)
"\n"
Inspect.name(print)
"\n"
Inspect.name(Int)
"\n"
Inspect.name(Int.Int)
"\n"