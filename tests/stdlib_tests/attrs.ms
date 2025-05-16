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