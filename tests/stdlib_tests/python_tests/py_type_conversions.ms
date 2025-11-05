import python.*

PythonObject(24).to_moss()
"\n"
PythonObject(true).to_moss()
"\n"
to_py(2.0).to_moss()
"\n"
builtins.str("lit 😎").to_moss()
"\n"
to_py(nil).to_moss()
"\n"
builtins.str([1, 2, true, [], nil]).to_moss()
"\n"
builtins.str([]).to_moss()
"\n"
builtins.str([[[42]]]).to_moss()
"\n"

v = builtins.list([1,2, [true, false], 3,4, [nil]])
v.to_moss()
"\n"

d1 = PythonObject({"a": 1, true: "hello", 42: {"a": 2}}).to_moss()
assert(d1 == {true: "hello", 42: {"a": 2}, "a": 1}, "d1")
d2 = PythonObject({:}).to_moss()
assert(d2 == {:}, "d2")
d3 = PythonObject({1: [1,2,3], 2:[{"2": true}]}).to_moss()
assert(d3 == {1: [1, 2, 3], 2: [{"2": true}]}, "d3")

builtins.str(builtins.set([1,2,3,4,5])).to_moss()
"\n"
builtins.set([11,22,33,22,11]).to_moss()
"\n"