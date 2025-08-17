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