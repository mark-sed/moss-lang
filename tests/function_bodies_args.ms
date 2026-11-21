fun foo(f) {
    return f(42)
}

foo(fun(a){
    c = a * 4
    return c + 2
})
"\n"

a = 4
c = a + (fun(a) {
    r = 4 + a
    return r
})(2)
c
"\n"

fun goo(i, j, a:Function=fun f(a:Int,b:Int) {
            c = a * 100
            d = b * 100
            r = 1..100
            return a + b + Math.sum(r)
        }, mult:Bool=false) {
    return a(i, j * (mult ? 2 : 1))
}

goo(1, 2)
"\n"

fun outter(f1=fun inner(a, f2=fun inner2(b) {
            return a
        }, show:Bool=true, otherf=fun()=nil) {
        return f2(a)
    }, a=8) {
    return f1(a)
}

outter(a=42)
"\n"

fun fg2(f3=fun argf(a) {
            return a + 4
        }) {
    return f3(2) + argf(2)
}

fg2()
"\n"

fun bar(f=fun(a, f=fun(a)=a+1, doit:Bool=false) {
            return f(a)
        }) {
    return f(1)
}

bar()
"\n"