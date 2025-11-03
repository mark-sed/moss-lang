import python.*

PythonObject(24).to_moss()
"\n"
PythonObject(true).to_moss()
"\n"
to_py(2.0).to_moss()
"\n"
stdlib.str("lit 😎").to_moss()
"\n"
to_py(nil).to_moss()
"\n"
stdlib.str([1, 2, true, [], nil]).to_moss()
"\n"
stdlib.str([]).to_moss()
"\n"
stdlib.str([[[42]]]).to_moss()
"\n"

v = stdlib.list([1,2, [true, false], 3,4, [nil]])
v.to_moss()
"\n"