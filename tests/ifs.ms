a = 6
if(a < 2) {
    a = 2
}
else a = 0

a ++ "\n"

fun cmp1(a) {
    if (a) {
        return "yes\n"
    }
    else {
        return "no\n"
    }
}

cmp1(true)
cmp1(false)

fun cmp2(a, b) {
    if (a == b) {
        return "yes\n"
    }
    else {
        return "no\n"
    }
}

cmp2(1, 2)
cmp2(2, 1)
cmp2("true", "true2")
cmp2("moss", "moss")

fun cmpT2(a, b) {
    if (a == b) return "yes\n"
}

cmpT2(1, 1)
cmpT2(2.0, 8.48e8)
"\n"

fun check(a:Int) {
    if (a < 30) {
        return "very small\n"
    }
    else if (a < 100) {
        return "small\n"
    }
    else if (a < 150) {
        return "medium\n"
    }
    else if (a < 200) {
        return "big\n"
    }
    else {
        return "very big\n"
    }
}

check(0)
check(45)
check(120)
check(199)
check(200)
check(20000)

if (check(0) == "big\n") {
    "a\n"
} else if (check(180) == "big\n") {
    "b\n"
}