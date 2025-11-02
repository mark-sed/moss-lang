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
cosf.call(2*Math.pi).to_moss()
"\n"
