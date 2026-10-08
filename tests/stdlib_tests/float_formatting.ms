(424.4).format("f")
"\n"
(424.4).format(".0f")
"\n"
(424.4).format(".1f")
"\n"
(424.4).format(".2f")
"\n"
(424.4).format(".6f")
"\n"
(424.4).format("10.2f")
"\n"
(424.4).format("010.2f")
"\n"
(424.4).format("-10.2f")
"\n"
(424.4).format("+10.2f")
"\n"
(424.4).format(" 10.2f")
"\n"
(424.4).format("#10.2f")
"\n"

(424.4).format("e")
"\n"
(424.4).format(".0e")
"\n"
(424.4).format(".2e")
"\n"
(424.4).format(".6e")
"\n"
(424.4).format("10.2e")
"\n"
(424.4).format("+10.2e")
"\n"

(424.4).format("E")
"\n"
(424.4).format(".2E")
"\n"
(424.4).format("+10.2E")
"\n"

(424.4).format("g")
"\n"
(424.4).format(".2g")
"\n"
(424.4).format(".4g")
"\n"
(424.4).format(".8g")
"\n"
(424.4).format("10.4g")
"\n"
(424.4).format("#.4g")
"\n"

(424.4).format("G")
"\n"
(424.4).format(".2G")
"\n"
(424.4).format(".8G")
"\n"

(424.4).format("a")
"\n"
(424.4).format(".2a")
"\n"
(424.4).format(".6a")
"\n"
(424.4).format("A")
"\n"
(424.4).format(".2A")
"\n"

(-424.4).format("f")
"\n"
(-424.4).format("+.2f")
"\n"
(-424.4).format("10.2f")
"\n"
(-424.4).format("010.2f")
"\n"
(-424.4).format("#.2f")
"\n"

(0.0).format("f")
"\n"
(0.0).format(".2f")
"\n"
(-0.0).format(".2f")
"\n"
(0.00000123).format(".2f")
"\n"
(123456789.0).format(".2f")
"\n"

a = 0.3
a.format("f")
"\n"
a.format(".1f")
"\n"
a.format(".2f")
"\n"
a.format(".10f")
"\n"
a.format("e")
"\n"
a.format(".3e")
"\n"
a.format("g")
"\n"
a.format(".3g")
"\n"

(123.456).format(".4g")
"\n"
(123456.0).format(".4g")
"\n"
(123456789.0).format(".4g")
"\n"
(0.00123456).format(".4g")
"\n"
(0.0000123456).format(".4g")
"\n"
(0.000000123456).format(".4g")
"\n"

(42.5).format("+8.2f")
"\n"
(42.5).format("-8.2f")
"\n"
(42.5).format("08.2f")
"\n"
(42.5).format(" 8.2f")
"\n"
(42.5).format("#8.2f")
"\n"
(42.5).format("+#8.2f")
"\n"
(42.5).format("+#08.2f")
"\n"
(-42.5).format("+#08.2f")
"\n"

fun try_f(v, f) {
    try
        v.format(f)
    catch (e:ValueError)
        return "caught\n"
}

try_f(424.4, "")
try_f(424.4, "%")
try_f(424.4, ".f")
try_f(424.4, "..2f")
try_f(424.4, "...")
try_f(424.4, ".")
try_f(424.4, "2")
try_f(424.4, ".2")
try_f(424.4, "q")
try_f(424.4, "d")
try_f(424.4, "s")
try_f(424.4, "x")
try_f(424.4, "n")
try_f(424.4, "p")

try_f(424.4, "f f")
try_f(424.4, ".2f f")
try_f(424.4, "f%s")
try_f(424.4, "f%d")
try_f(424.4, "f%n")
try_f(424.4, "hello f")
try_f(424.4, "f hello")
try_f(424.4, "hello")
try_f(424.4, "%%")
try_f(424.4, "%f")
try_f(424.4, "%.2f")
try_f(424.4, "%s")
try_f(424.4, "%p")


try_f(424.4, "1$f")
try_f(424.4, "2$f")
try_f(424.4, "Lf")
try_f(424.4, "lld")

try_f(424.4, "{:.2f}")
try_f(424.4, "$(echo hacked)")
try_f(424.4, "`echo hacked`")
try_f(424.4, "; rm -rf /")
try_f(424.4, "f; rm -rf /")