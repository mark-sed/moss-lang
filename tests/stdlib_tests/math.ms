Int(Math.pi) - Int(Math.e)
"\n"
Int(Math.tau) - Int(Math.phi)
"\n"
Int(Math.euler_gamma)
"\n"
Int(Math.sqrt(4) + Math.sqrt(36))
"\n"

fun f_test(f, x) {
    v = f(x)
    return String(v)[0..4] ++ "\n"
}

f_test(Math.sin, 0)
f_test(Math.sin, Math.pi/6)
f_test(Math.sin, Math.pi)
f_test(Math.sin, -3 * Math.pi / 2)
f_test(Math.sin, -8 * Math.pi)
f_test(Math.sin, 42)

f_test(Math.cos, 0)
f_test(Math.cos, Math.pi/3)
f_test(Math.cos, Math.pi/2)
f_test(Math.cos, Math.pi)
f_test(Math.cos, Math.pi * 4)
f_test(Math.cos, 42)

f_test(Math.tan, 0)
f_test(Math.tan, Math.pi/4)
f_test(Math.tan, Math.pi)
f_test(Math.tan, -Math.pi)
f_test(Math.tan, Math.pi * 4)
f_test(Math.tan, 42)