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