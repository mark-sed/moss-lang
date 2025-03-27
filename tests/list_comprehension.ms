[c : c = [1,2,3]]
"\n"
l1 = [1, 2, 3]
l2 = [4, 5, 6]
zipped = [[x, y] : x = l1, y = l2]
zipped
"\n"

[a : a = (1,3..100)] // odd numbers from 1 to 99
"\n"

fun all(v) {
    for (x: v) {
        if (not x) return false
    }
    return true
}

[p if(all([p % x != 0 : x = [v : v = (2..Int(p^0.5) + 1)]])) : p = (2..100)] // primes
"\n"

greet = "Hello there programmer!"
[c if(c != " ") else "_" : c = greet] // "Hello_there_programmer!"
"\n"
[(c != " ") ? c : "_" : c = greet] // "Hello_there_programmer!"
"\n"

z = [[x, y] if (x > 0) else [0, 0]: x = [1, 2, 3], y = ["a", "b"]]
z
"\n"