import md_parser.*

f = open(args[0], "r")
lines = f.readlines()

parser = MDParser("\n".join(lines))
parsed = parser.parse()

for (p: parsed) {
    p
    "\n"
}

"\n"
for (p: parsed) {
    p.type
    if (type(p.value) == List) {
      ": ["
      first = true
      for (v: p.value) {
        if (not first)
          ", "
        v.type
        first = false
      }
      "]"
    }
    "\n"
}
