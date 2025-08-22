class MyInt : Int {

}

mi = MyInt(42)
type(mi)
"\n"
mi.__value
"\n"
mi2 = MyInt("10")
mi2.__value
"\n"

F = Float
class MyFloat : (F) {

}

mf = MyFloat(0.5)
type(mf)
"\n"
mf.__value
"\n"

class MyBool : Bool {

}

mb = MyBool(true)
type(mb)
"\n"
mb.__value
"\n"