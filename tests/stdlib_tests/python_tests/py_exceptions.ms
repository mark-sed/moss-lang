import python

r = python.module("random")
try {
    try {
        r.def("NONEXISTENT_FUN")
    } catch (e:python.PythonException) {
        e.obj.get("msg")
    } finally {
        "Ran!\n"
    }
} catch(e:python.PythonException) {
    e.msg++"\n"
}