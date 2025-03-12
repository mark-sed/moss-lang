Int(Math.pi) - Int(Math.e)
"\n"
Int(Math.tau) - Int(Math.phi)
"\n"
Int(Math.euler_gamma)
"\n"
Int(Math.sqrt(4) + Math.sqrt(36))
"\n"

fun sin_test(x) {
    v = Math.sin(x)
    return String(v)[0..4] ++ "\n"
}

sin_test(0)
sin_test(Math.pi/6)
sin_test(Math.pi)
sin_test(-3 * Math.pi / 2)
sin_test(-8 * Math.pi)
sin_test(42)
