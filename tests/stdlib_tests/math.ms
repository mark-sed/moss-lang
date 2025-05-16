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

round(3.1)
"\n"
round(3.51)
"\n"
round(0, 3)
"\n"
round(-100.123, 0)
"\n"
round(12)
"\n"
af = 7.71256
round(af, 4)
"\n"
round(-0.123, 2)
"\n"

Math.abs(1)
"\n"
Math.abs(-123.45)
"\n"
Math.abs(0)
"\n"
Math.abs(-0.0)
"\n"
Math.abs(-42)
"\n"
Math.abs(Float("-inf"))
"\n"
Math.abs(Float("inf"))
"\n"
Math.abs(Float("-nan"))
"\n"
Math.abs(Float("nan"))
"\n"

lst1v = [1,2,3,4]
Math.sum(lst1v, start=0)
"\n"
Math.sum([-1,-2,1,2], 5)
"\n"
Math.sum([])
"\n"
Math.sum([1.3, Float("inf"), 7, -8])
"\n"

class MyList {
    fun MyList(x) {
        this.x = x
    }

    fun __iter() = this.x
}

mls = MyList([10,20,30,40])
Math.sum(mls, -10)
"\n"

lshift(1, 4)
"\n"
lshift(-4, 5)
"\n"
rshift(100, 3)
"\n"
rshift(1, 10)
"\n"

Math.log(8, 2)
"\n"

fun ilog(x, base) = round(Math.log(x, base)) ++ "\n"

ilog(1024, 2)
ilog(1.0/8, 2)
ilog(1, 4)
ilog(0.01, 10)

Math.divmod(5, 2)
"\n"
Math.divmod(2500, 42)
"\n"
try
    Math.divmod(7, 0)
catch (e:DivisionByZeroError)
    "caught\n"

r, q = Math.divmod(5.4, 2.3)
r
"\n"
q
"\n"
Math.divmod(2.3, 45)
"\n"