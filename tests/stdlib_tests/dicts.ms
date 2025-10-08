fun order(d) {
    vals = []
    for (k, v: d) {
        ~vals.append([k, v])
    }
    n = vals.length()
    for (i: 0..n) {
        min_idx = i
        for (j: i + 1.. n) {
            if (String(vals[j][0]) < String(vals[min_idx][0])) {
                min_idx = j
            }
        }
        v = vals[i]
        vals[i] = vals[min_idx]
        vals[min_idx] = v
    }
    "{"
    if (vals.empty())
        ":"
    first = true;
    for (k, v: vals) {
        if (not first)
            ", "
        first = false
        if (type(k) == String)
            "\""++k++"\""
        else
            k
        ": "
        if (type(v) == String)
            "\""++v++"\""
        else
            v
    }
    "}\n"
}

dc1 = Dict([["A", 1], ["B", 2], ["C", 3]])
~order(dc1)
~dc1.clear()
dc1
"\n"
dc2 = {"a": 1, "b": {"a": 2, "c": 3}, "c":nil}
~dc2.clear()
dc2
"\n"
dc3 = {:}
~dc3.clear()
dc3
"\n"

dc1 = {"a": 1, "b": 2, "c": 3}
~order(dc1)
dc1.pop("b")
"\n"
~order(dc1)
dc1.pop("a")
"\n"
~order(dc1)
dc1.pop("c")
"\n"
~order(dc1)
try
    dc1.pop("a")
catch (e:KeyError)
    "caught\n"
dc1.pop("a", nil)
"\n"

dc2 = {"hello": [1,2,3], "dct": {"a": 1}}
dc2["dct"].pop("a")
"\n"
assert(dc2 == {"hello": [1, 2, 3], "dct": {:}})
dc2.pop("dct")
"\n"
~order(dc2)

dc4 = {1: true, 2: false, 3: {"a": nil}}
~dc4.remove(3)
dc4
"\n"
~dc4.remove(1)
~dc4.remove(2)
~order(dc4)
try {
    dc4.remove(1)
} catch (e:KeyError)
    "caught\n"

dc5 = {1: true, 2: false, 3: {"a": nil}}
dc5.get(1)
"\n"
dc5.get(8.56)
"\n"
dc5.get(false, "not in\n")

Dict.pop("a", {"a": 1})
"\n"
try
    Dict.pop("a", 24)
catch (e:ValueError)
    "caught\n"

Dict.get("a", nil, {"a": 1})
"\n"
try
    Dict.get("a", nil, 24)
catch (e:ValueError)
    "caught\n"

dc6 = {1:1, 2:2, 3:3, 4:4}
dc6.length()
"\n"
dc6.empty()
"\n"
{:}.empty()
"\n"
{:}.length()
"\n"
dc7 = {1: [1,2,3], nil: {"a": 1, "v": 2}, 3: "fsd"}
dc7.length()
"\n"

dc8 = {"a": 1}
dc9 = dc8.copy()
dc9["a"] = 2
dc8
"\n"
dc9
"\n"
assert(id(dc8) != id(dc9), "ids are the same after copy")
d10 = {1: 2, 2: {"a": 4}}
d11 = d10.copy()
d11[1] = nil
d11[2]["a"] = "changed"
d10[1]
"\n"
d11[1]
"\n"
~order(d10[2])
~order(d11[2])

its = {"a": 1, "b": 2, "c": 3}.items()
assert(its.length() == 3, "size")
assert(["a", 1] in its, "a1")
assert(["b", 2] in its, "b2")
assert(["c", 3] in its, "c3")

{:}.items()
"\n"

its2 = {"dct": {"sub": true}, true: nil}.items()
assert(its2.length() == 2, "size")
assert(["dct", {"sub": true}] in its2, "sub")
assert([true, nil] in its2, "true")

its3 = {"lst": [1, 2, 3]}.items()
its3
"\n"

ks = {"a": 1, "b": 2, "c": 3}.keys()
assert(ks.length() == 3, "size")
assert("a" in ks, "a")
assert("b" in ks, "b")
assert("c" in ks, "c")

{:}.keys()
"\n"

ks2 = {true: [1,2], false: "f"}
assert(ks2.length() == 2, "size")
assert(true in ks2, "tr")
assert(false in ks2, "fl")

vl = {"a": 1, "b": 2, "c": 3}.values()
assert(vl.length() == 3, "size")
assert(1 in vl, "1")
assert(2 in vl, "2")
assert(3 in vl, "3")

{:}.values()
"\n"

vl2 = {1: {"a": "a"}, 2: [1,2], 3:nil}.values()
assert(vl2.length() == 3, "size")
assert({"a": "a"} in vl2, "aa")
assert([1,2] in vl2, "12")
assert(nil in vl2, "nil")

dc = {"a": 1, "b": 2, "c": 3}
i1 = dc.pop_item()
i2 = dc.pop_item()
i3 = dc.pop_item()
assert(dc.length() == 0, "size")
assert(Dict([i1, i2, i3]) == {"a": 1, "b": 2, "c": 3}, "Diff")

{1: true}.pop_item()
"\n"

try {
    {:}.pop_item()
} catch(k: KeyError)
    "caught\n"

assert(Dict.from_keys([1,2,3]) == {1:nil, 2:nil, 3:nil}, "diff1")
assert(Dict.from_keys([1,2,3], "h") == {1:"h", 2:"h", 3:"h"}, "diff2")
assert(Dict.from_keys([]) == {:}, "diff3")

class It {
    fun It(end) {
        this.end = end
    }

    fun __iter() {
        this.i = 0
        return this
    }

    fun __next() {
        if (this.i >= this.end)
            raise StopIteration()
        v = this.i
        this.i += 1
        return v
    }
}

assert(Dict.from_keys(It(3)) == {0:nil, 1:nil, 2:nil}, "diff4")
assert(Dict.from_keys(It(3), 1) == {0:1, 1:1, 2:1}, "diff5")

assert(Dict.from_keys(10..7, 1) == {10:1, 9:1, 8:1}, "diff6")

try
    Dict.from_keys([[1,2]])
catch (e:TypeError)
    "caught\n"

try
    Dict.from_keys(1)
catch (e:TypeError)
    "caught\n"