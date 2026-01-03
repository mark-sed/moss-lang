import re

r = re.compile("HELLO", re.REFlags(icase=true))
m = r.match("hello")
m.groups
"\n"

m = re.search("([a-z]*)|([a-z]*)", "abc|def", re.REFlags(nosubs=true, optimize=true))
m.groups
"\n"

// TODO: Enable once locale can be set
//m = re.search("[a-d]", "otče", re.REFlags(collate=true, grammar=re.REFlags.Grammars.basic))
//m.groups
//"\n"

m = re.search("(^error)", "info: first\nerror: second\ninfo: third",
              re.REFlags(multiline=false))
m
"\n"
m = re.search("(^error)", "info: first\nerror: second\ninfo: third",
              re.REFlags(multiline=true))
m.groups
"\n"

m = re.search(r"(\d+)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.ECMAScript))
m.groups
"\n"
m = re.search(r"(\d+)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.basic))
m
"\n"
m = re.search(r"(\d+)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.extended))
m
"\n"
m = re.search(r"(abc|123)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.ECMAScript))
m
"\n"
m = re.search(r"(abc|123)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.extended))
m
"\n"
m = re.search(r"(abc|123)", "abc123", re.REFlags(grammar=re.REFlags.Grammars.basic))
m
"\n"

m = re.search(r"abc|123", "abc123", re.REFlags(grammar=re.REFlags.Grammars.awk))
m
"\n"

m = re.match(r"a?b", "ab", re.REFlags(grammar=re.REFlags.Grammars.ECMAScript))
m
"\n"
m = re.match(r"a?b", "ab", re.REFlags(grammar=re.REFlags.Grammars.awk))
m
"\n"
m = re.match(r"a\?b", "a?b", re.REFlags(grammar=re.REFlags.Grammars.grep))
m
"\n"
