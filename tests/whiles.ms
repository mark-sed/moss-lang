while(false) {
    "ERROR!"
}

fun wh(count) {
    while(count > 0) {
        "."
        count -= 1;
    }
    "done\n"
}

a = wh(0)
a = wh(1)
a = wh(5)

count = 3
while(count > 0) {
    ","
    count = count - 1
}
"done\n"

do {
    "-\n"
} while(false)

i = 0
do {
    i += 1
    i++"\n"
} while(i <= 2)

do {} while(false)

do i++"\n"; while(i < 0)