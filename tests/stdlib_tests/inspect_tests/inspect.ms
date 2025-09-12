import inspect

fun foo(a:Int, b:Bool, c=nil) {}

inspect.signature(foo)
"\n"

fun foo2(a="hello", ... other) {
}

inspect.signature(foo2)
"\n"

fun foo(a) {}

try
    inspect.signature(foo)
catch (e:TypeError)
    "caught\n"

class Animal {
    fun Animal(name:String, age:[Int,Float]=0) {
        this.name = name;
        this.age = age;
    }
}

inspect.signature(Animal.Animal)
"\n"

inspect.signature(fun(a, b=1, c:Bool=false)=a)
"\n"

inspect.signature(fun lambda_with_name()=4)
"\n"

enum En1 {A;}
class Cl2 {}
space Spc {}

inspect.name(En1)
a = Cl2
"\n"
inspect.name(a)
"\n"
inspect.name(Spc)
"\n"
inspect.name(print)
"\n"
inspect.name(Int)
"\n"
inspect.name(Int.Int)
"\n"

enum EnB {
    A, B, C, D, E
}

inspect.enum_values(EnB)
"\n"
vl = inspect.enum_values(EnB)
type(vl[0])
"\n"

enum EmptyEnum {}
inspect.enum_values(EmptyEnum)
"\n"