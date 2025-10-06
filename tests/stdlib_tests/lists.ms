[1,2].length()
"\n"
[].length()
"\n"
lst = [1,2,3,4,0,0.5, true, nil]
lst.length()
"\n"

try
    List.length(5)
catch (e:ValueError)
    "caught\n"

l_0 = [1,2,3]
l_0
"\n"
~l_0.append(4)
~l_0.append([true, nil])
l_0
"\n"
~l_0.append(9, [], true)
l_0
"\n"

l_1 = [1, true, 2, false]
l_1
"\n"
l_1.pop()
"\n"
l_1.pop(1)
"\n"
l_1
"\n"

l_2 = [1,2,3,4,5,6]
l_2
"\n"
l_2.pop(-2)
"\n"
l_2.pop(-3)
"\n"
l_2
"\n"

[1,2,3,4].reverse()
"\n"
[].reverse()
"\n"
[true, false].reverse()
"\n"
[1,2,3,-4,-5][0..3].reverse()
"\n"

[1,2,3].__iter()
"\n"
[1,2,3].__next()
"\n"

ls1 = [1,2,3,4,[3,4,5,[]]]
ls1
"\n"
~ls1.clear()
ls1
"\n"

ls2 = []
~ls2.clear()
ls2
"\n"

[1,2,3].empty()
"\n"
[].empty()
"\n"
ls4 = [[[]]]
ls4.empty()
"\n"
ls4[0].empty()
"\n"
ls4[0][0].empty()
"\n"

ls5 = [1,2,3,4,5]
ls6 = ls5.copy()
~ls6.append(6)
ls5
"\n"
ls6
"\n"
ls7 = [[1,2,3]]
ls8 = ls7.copy()
ls8[0][0] = 5
ls7
"\n"
ls8
"\n"

[1,2,3,1,2,3].count(1)
"\n"
["1", "hello", 1, 2, "1", "01"].count("1")
"\n"
[].count(1000)
"\n"
[[1, 2],[1,2,3],nil].count([1,2])
"\n"
[[],[[]],[],[]].count([])
"\n"
[true, false, true].count(1)
"\n"

[1,2,3].index(2)
"\n"
try
    [].index(true)
catch(e:ValueError)
    "caught\n"

class E {
    fun E(v) {
        this.v = v
    }

    fun (==)(e2) {
        return this.v == e2.v
    }
}

e = E("")
[e, TypeError("")].index(e)
"\n"

try
    [E(1), 2].index(e)
catch (e:ValueError)
    "caught\n"

try
    [1,2,3,4,5].index(1, 1, 4)
catch (e:ValueError)
    "caught\n"

[1,2,3,4,5].index(3, 1, 4)
"\n"

l1 = [1,2,3]
~l1.insert(0, 0)
l1
"\n"

~l1.insert(l1.length(), 4)
l1
"\n"

l2 = [1,3]
~l2.insert(1, true)
l2
"\n"

l3 = []
~l3.insert(0, nil)
l3
"\n"

~l3.insert(-10, 1)
l3
"\n"

~l3.insert(100, 2)
l3
"\n"