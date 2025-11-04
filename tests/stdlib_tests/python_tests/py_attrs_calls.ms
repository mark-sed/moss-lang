import python

r = python.module("random")
randint = r.get("randint")
randint
"\n"

m = python.module("math")
cosf = m.get("cos")
res = cosf.call(m.get("pi").to_moss())
res.to_moss()
"\n"
cosf.call_moss(m.get("pi"))
"\n"
cosf.call_moss(2*Math.pi)
"\n"

~m.def("sin")
m.sin.call_moss(Math.pi)
"\n"

try {
    r.def("NONEXISTENT_FUN")
} catch (e:python.PythonException) {
    e
    e.obj.get("args")
    "\n"
    python.builtins.get("str").call(e.obj)
    "\n"
}

import python.*

math = module("math", populate=true)
math.gcd(180, 980).to_moss()
"\n"