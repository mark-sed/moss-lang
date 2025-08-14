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
        this.goo() ++ "\n"
    }
}

foo1 = Foo1()
foo1.NAME ++ "\n"

class MyOutterClass {
    class MyInnerClass {
        fun MyInnerClass(x) {
            this.x = x
        }

        fun goo(a) {
            return a + this.x
        }
    }
}

moc = MyOutterClass.MyInnerClass
mic = moc(5)
mic.goo(5)
"\n"

mic1 = MyOutterClass.MyInnerClass(56)
mic1.x
"\n"
mic1.goo(5)
"\n"

space MyOutterSpace {
    class MyInnerClass {
        fun MyInnerClass(x) {
            this.x = x
        }
    }
}

mic2 = MyOutterSpace.MyInnerClass(42)
mic2.x
"\n"