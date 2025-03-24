a = "Hi there\n"
i = 1
v = "ello"
a
a[i] = v
a
i = -12
inst = "> " ++ a[0] 
a[i] = inst
a
a[-1] = "!\n"
a

space D {
    vals = [1,2,3]
}

D.vals
i = 1
lv = true
D.vals[i] = lv
"\n"
D.vals
"\n"
i = -1
D.vals[i] = lv
D.vals
"\n"
D.vals[-3] = false
D.vals
"\n"

txt = "Val: ?"
txt[-1] = 1
txt ++ "\n"
txt[-1] = Int
txt ++ "\n"

class Test {
    fun Test() {
        this.vals = [1,2,3,4]
    }

    fun set(i, v) {
        this.vals[i] = v
    }

    fun get_vals() = this.vals
}

t = Test()
t.vals
"\n"
~t.set(0, -8)
t.vals
"\n"