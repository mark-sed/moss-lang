dc8 = {"a": 1}
dc9 = copy(dc8)
dc9["a"] = 2
dc8
"\n"
dc9
"\n"
d10 = {1: 2, 2: {"a": 4}}
d11 = copy(d10)
d11[1] = nil
d11[2]["a"] = "changed"
d10
"\n"
d11
"\n"

ls5 = [1,2,3,4,5]
ls6 = copy(ls5)
~ls6.append(6)
ls5
"\n"
ls6
"\n"
ls7 = [[1,2,3]]
ls8 = copy(ls7)
ls8[0][0] = 5
ls7
"\n"
ls8
"\n"

class SomeCls {
    fun SomeCls(x, a, copy_a:Bool=true) {
        this.x = x 
        if (copy_a)
            this.a = copy(a)
        else
            this.a = a
    }

    fun __String() {
        return this.x ++ ", " ++ this.a ++ "\n"
    }
}

lst = [1,2,3]
sc1 = SomeCls(5, lst)
sc2 = copy(sc1)
sc2.x = -1
sc1
sc2
lst[0] = false
sc1
sc2

lst2 = [1,2,3]
sc3 = SomeCls(5, lst2, false)
sc4 = copy(sc3)
sc4.x = -1
sc3
sc4
lst2[0] = false
sc3
sc4

fun foo() {
    return 4
}

a = foo
b = copy(a)

assert(id(a) == id(b), "fun copied")

fun foo(a) = a

c = foo
d = copy(c)

assert(id(a) == id(b), "fun list copied")

enum SomeEn {A, B, C;}

e = copy(SomeEn)
assert(id(SomeEn) == id(e), "enum copied")
assert(id(SomeEn.A) == id(copy(SomeEn.A)), "enum val copied")

class A {
    NAME = "A"
}

a_copy = copy(A)
a_copy.NAME = "copy of A"
A.NAME
"\n"
a_copy.NAME
"\n"
assert(id(a_copy) == id(A), "copied class")

class B : A {
    fun B() {
        s = super
        s2 = copy(super)
        assert(id(s) != id(s2), "did not copy super")
    }
}

b_obj = B()
b_copy = copy(b_obj)
b_copy.a = 44
try
    b_obj.a
catch (e:AttributeError)
    "caught\n"
assert(id(b_obj) != id(b_copy), "did not copy objects")

i1 = 42
i2 = copy(i1)
assert(id(i1) == id(i2), "copied int")

f1 = 42.0
f2 = copy(f1)
assert(id(f1) == id(f2), "copied float")

b1 = true
b2 = copy(b1)
assert(id(b1) == id(b2), "copied bool")

s1 = "hi"
s2 = copy(s1)
assert(id(s1) == id(s2), "copied string")

n1 = nil
n2 = copy(n1)
assert(id(n1) == id(n2), "copied nil")

space SomeSpace1 {
    NAME = "Some space1"
}

sp1 = SomeSpace1
sp2 = copy(sp1)
sp2.NAME = "changed\n"
sp1.NAME
sp2.NAME
assert(id(sp1) == id(sp2), "copied space")

n1 = md"hi"
n2 = copy(n1)
assert(id(n1) == id(n2), "copied note")

import sys
mod1 = sys
mod2 = copy(mod1)
assert(id(mod1) == id(mod2), "copied module")