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

/* Uncomment once fixed
r4 = Range(0, 30)
r4.__next()
"\n"
r4.__next()
"\n"
r4.__next()
"\n"
*/

r5 = Range(0, 3, 1)
r5.__next()
"\n"
r5.__next()
"\n"
try {
    r5.__next()
} catch (e:StopIteration) {
    
} finally {
    "\n"
}