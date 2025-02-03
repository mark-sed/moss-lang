class Animal {

}

class Hungry {

}

class Cat : Animal, Hungry {

    CNAME = "Cat"
    
    fun Cat(name:String) {
        this.name = name
    }

    fun get_name() {
        return this.name
    }

    fun sound() {
        return "meow\n"
    }

    4*10+2
    "\n"
}

Cat++"\n"

vilda = Cat("Vilda")
vilda
"\n"
vilda.get_name()
"\n"
vilda.sound()

// Test implicit constructor
anm = Animal()
anm
"\n"

class Foo1 {
    NAME = "foo1"

    fun goo() {
        return NAME
    }

    fun Foo1() {
        goo() ++ "\n"
    }
}

foo1 = Foo1()
foo1.NAME ++ "\n"
