import re

r = re.compile("HELLO")
r
"\n"
r.__regex
"\n"
r.pattern
"\n"

m = r.match("HELLO")
m
"\n"
m.groups
"\n"

m2 = re.search(r"((\w+)-(\d+))", "abc-123")
if (Bool(m2))
    m2.groups
"\n"

re.match(r"\d+", "abc123")
"\n"
gs = re.search(r"\d+", "abc123").groups
gs
"\n"
gs[0]
"\n"