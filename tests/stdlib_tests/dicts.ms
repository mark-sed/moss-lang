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