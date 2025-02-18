fun print_range(rng:Range) {
    "["
    first = true
    run = true
    while(run) {
        try {
            v = rng.__next()
            if (not first)
                ", "
            v
        } catch (e:StopIteration) {
            "]\n"
            run = false
        }
        first = false
    }
}

a = 0..3
~print_range(a)
b = 0,2..8
~print_range(b)
~print_range(b)

v1 = -2
v3 = 2
~print_range((v1..v3))

v2 = 0
c = v1,v2..v3
~print_range(c)

~print_range((v1-2, v2-2..v3+2))

~print_range((1..-4))
~print_range((1,10..2))

~print_range((10,10+1..2))

~print_range((v3,4..7))
~print_range((0,v3..v3*3))
v4 = -104
~print_range((-100,-101..v4))

fun get_v(i) { return i; }

~print_range((get_v(0), get_v(1)..5))