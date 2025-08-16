dc1 = Dict([["A", 1], ["B", 2], ["C", 3]])
dc1
"\n"
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
dc1
"\n"
dc1.pop("b")
"\n"
dc1
"\n"
dc1.pop("a")
"\n"
dc1
"\n"
dc1.pop("c")
"\n"
dc1
"\n"
try
    dc1.pop("a")
catch (e:KeyError)
    "caught\n"
dc1.pop("a", nil)
"\n"

dc2 = {"hello": [1,2,3], "dct": {"a": 1}}
dc2["dct"].pop("a")
"\n"
dc2
"\n"
dc2.pop("dct")
"\n"
dc2
"\n"

dc4 = {1: true, 2: false, 3: {"a": nil}}
~dc4.remove(3)
dc4
"\n"
~dc4.remove(1)
~dc4.remove(2)
dc4
"\n"
//try {
//    dc4.remove(1)
//} catch (e:KeyError)
//    "caught\n"

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