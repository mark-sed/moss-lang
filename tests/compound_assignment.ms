class A {
    fun A() {
        this.i = "I say "
        this.v = 2
    }
}

a = A()
c = "hi"
a.i ++= c
a.i
"\n"
a.i ++= "!"
a.i
"\n"

class B {
    fun B(a) {
        this.a = a
    }
}

b = B(a)
b.a.i ++= "?\n"
a.i

a.v
"\n"
a.v ^= 2
a.v
"\n"

a.v += 10
a.v
"\n"

a.v -= 5
a.v
"\n"

a.v /= 3
a.v
"\n"

b.a.v *= 2
a.v
"\n"

a.v %= 4
a.v
"\n"

l_str = ["a", "b", "c"]
l_str
"\n"
l_str[0] ++= l_str[1]
l_str[1] ++= "d"
l_str
"\n"

l_int1 = [1, 2, 3]
l_int1
"\n"
l_int1[1] ^= l_int1[2]
l_int1[2] ^= 0
l_int1
"\n"

l_int1
"\n"
l_int1[0] += l_int1[1]
l_int1[2] += -1
l_int1
"\n"

l_int1
"\n"
l_int1[0] -= l_int1[0]
l_int1[1] -= 8
l_int1
"\n"

l_int2 = [1, 5, 10]
l_int2
"\n"
l_int2[2] /= l_int2[1]
l_int2[0] /= 2.0
l_int2
"\n"

l_int2
"\n"
l_int2[2] *= l_int2[1]
l_int2[0] *= 2
l_int2
"\n"

l_int2
"\n"
l_int2[2] %= l_int2[1]
l_int2[0] %= 2
l_int2
"\n"
