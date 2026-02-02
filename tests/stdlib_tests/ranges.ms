r = Range(1, 100, 2)
r.__next()
"\n"
r.__next()
"\n"
r.__next()
"\n"

r2 = Range(100, 0)
r2.__next()
"\n"
r2.__next()
"\n"
r2.__next()
"\n"

r3 = Range(-2, 30, 5)
r3.__next()
"\n"
r3.__next()
"\n"
r3.__next()
"\n"

r4 = Range(0, 30)
r4.__next()
"\n"
r4.__next()
"\n"
r4.__next()
"\n"

r5 = Range(0, 3, 1)
r5.__next()
"\n"
r5.__next()
"\n"
r5.__next()
"\n"
try {
    r5.__next()
} catch (e:StopIteration) {
    "end"
} finally {
    "\n"
}

r6 = Range(0, -5, 2)
try {
    r6.__next()
} catch (e:StopIteration) {
    "end\n"
}

r6 = Range(0, -5, 2)
try {
    r6.__next()
} catch (e:StopIteration) {
    "end\n"
}

r7 = Range(5)
r7.__next()
"\n"
r7.__next()
"\n"
r7.__next()
"\n"

r8 = Range(-5)
r8.__next()
"\n"
r8.__next()
"\n"
r8.__next()
"\n"

r = 1..4
for (a: r) {
    for (b: r) {
        a++b++"|"
    }
}
"\n"

splitted_all = "aaaaaaa"
start = 1
v = true
try
    splitted = splitted_all[start..v]
catch (e:ValueError)
    "caught\n"

try
    splitted = splitted_all[v..10]
catch (e:ValueError)
    "caught\n"

try
    splitted = splitted_all[0,v..10]
catch (e:ValueError)
    "caught\n"

