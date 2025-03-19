if (true || foo()) {
    "true\n"
}

false || 1 > 4 || true || foo2()
"\n"

f = false
t = true

f || f || "hi\n"

fun some_f(b) = b

some_f(false) || some_f(true) && "there\n"

false && foo()
"\n"

fun foo_false() { return false; }

foo_false() || "fail\n"

fun file_exists(s) = s == "f.txt"

~file_exists("f.txt") && [print("exists!"), true][1] || print("does not exist.")
~file_exists("f2.txt") && [print("exists!"), true][1] || print("does not exist.")
