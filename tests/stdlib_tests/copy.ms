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