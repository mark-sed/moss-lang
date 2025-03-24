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

try a[-20] = 4; catch(e:IndexError) "caught\n"

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

try D.vals[20] = 4; catch(e:IndexError) "caught\n"

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

    fun __setitem(index, value) {
        this.vals[index] = value
    }
}

t = Test()
t.vals
"\n"
~t.set(0, -8)
t.vals
"\n"
t[1] = -7
t[2] = -6
t.vals
"\n"

class Test2 {

}

try {
    Test2()[2] = 5
} catch(e:NameError) "caught\n"

class Test3 {
    fun Test3() {
        this.vals = [nil, 1, 2]
    }

    fun __setitem(index:Bool, value) {
        this.vals[0] = value
    }
}

t3 = Test3()
t3.vals
"\n"
t3[true] = 0
t3.vals
"\n"
try
    t3[2] = false
catch(e:TypeError)
    "caught\n"