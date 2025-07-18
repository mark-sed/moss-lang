a, b, c, d = [1,2,3,4]
~print(a, b, c, d)

class SomeIterClass {
    fun SomeIterClass(x) {
        this.x = x
    }

    fun __iter() = this.x
}

e, f, g = SomeIterClass([true, nil, 5])
~print(e, f, g)

class SomeNonIter {}
try
    z, h = SomeNonIter()
catch (e:TypeError)
    "caught\n"

h, i = {true: 4, false: 0}
k1, v1 = h
k2, v2 = i
~print(k1, v1, k2, v2)

try
    j, k, ...l = [1,2]
catch (e:ValueError)
    "caught\n"

j, k, l, ...m = [1,2,3,4,5,6,7]
~print(j, k, l, m)

...n, o, p = [0, 1, 2, 3, 4, 5, 6, 7, 8]
~print(n, o, p)

q, r, ...s, t, u, v = ["q", "r", 3, 4, 5, 6, 7, 8, "t", "u", "v"]
~print(q, r, s, t, u, v)

w, ...x = [1]
~print(w, x)

...z, a1 = [2]
~print(z, a1)

b1, c1, ...d1, e1, f1 = SomeIterClass([1, 2, 3, 4, 5, 6, 7, 8, 9])
~print(b1, c1, d1, e1, f1)

g1, h1, ...i1 = SomeIterClass(["hi", "there"])
~print(g1, h1, i1)

for (a, b, ...c : [[1, 2], [3, 4], [5, 6]]) {
    ~print(a, b)
}

class F {
    fun F(x) {
        this.x = x
    }

    fun __iter() {
        return this.x
    }

}

for (a, b : [F([1, 2]), F([3, 4]), F([5, 6])]) {
    ~print(a, b)
}

dict1 = {"a": 1, "b": 2, "c": 3}
dict2 = {:}
for (k, v : dict1) {
    dict2[k] = v
}
assert(dict1 == dict2, "not equal dicts") 

for (a, ...b, c, d : [["a", 2, 3, 4, 5, "c", "d"], ["a", 2, 3, 4, 5, "c", "d"]]) {
    ~print(a, b, c, d)
}

for (...a, b : [[1], [2]]) {
    ~print(a, b)
}

for (m, ...n : []) {
    ~print(m, n)
}

try 
    for (m, ...n : [[]]) {
        ~print(m, n)
    }
catch (e:ValueError)
    "caught\n"