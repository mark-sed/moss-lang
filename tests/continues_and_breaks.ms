while(true) {
    "hi\n"
    break
}

i = 0
while (i < 10) {
    i += 1
    if (i % 2 == 0)
        continue
    i
}
"\n"

for (j : 1..100) {
    if (j == 10) break
    if (j % 2 == 0) continue
    j
}
"\n"

for(j:0..0x5) {
    continue
    "a\n"
}

for (c : 1..10) {
    for (d : 1..4) {
        if (d % 2 == 0) continue
        "d"++d++" "
    }
    "c"++c
}
"\n"

while(true) {
    while(true) {
        while(true) {
            for(i:"Hi\na") {
                if (i == "a") continue
                i
            } 
            break
        }
        break
    }
    break
}

do {
    break
    "aa"
} while (true)

o = 1
do {
    o += 1
    if (o >= 10) break
    if (o % 2 == 1) continue
    o
} while(true)
"\n"

a = 5
do {
    a -= 1
    a
    continue
} while (a > 0)
"\n"

a = 1
b = 2
do {
    a += 1
    a ++ " + ("
    do {
        b += 1
        if (b > 10) break
        if (b %2 == 0) continue
        b++" ["++a+b ++ "], "
    } while (true)
    ")|"
    if (a > 6) break
} while(true)
"\n"