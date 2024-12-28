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

~wh(0)
~wh(1)
~wh(5)

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