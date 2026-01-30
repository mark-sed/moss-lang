for (a : "Hello\n") {
    a
}

msg = "hi\n"
for (i : msg) i
for (i : msg) i

for (c : [1,2,3]) c
"\n"
iter = 0
some_vals = [2, 6, 8]
for (iter: some_vals) iter
iter
"\n"

class MyList {
    fun MyList(lst) {
        this.lst = lst
    }
}

lst = MyList([1,2,3])

try {
    for (l: lst) l
} catch (e:TypeError) {
    "no __next\n"
}


class MyIterable {
    fun MyIterable(lst) {
        this.lst = lst
    }

    fun __iter() {
        return this.lst
    }
}

lst2 = MyIterable([1,2,3,4,5])
for (i1: lst2) i1
"\n"
for (i2:lst2) i2
"\n"


class MyIterator {
    fun MyIterator(lst) {
        this.lst = lst
        this.i = 0
    }

    fun __iter() {
        this.i = 0
        return this
    }

    fun __next() {
        if (this.i > 3)
            raise StopIteration()
        v = this.lst[this.i]
        this.i += 1
        return v
    }
}

lst3 = MyIterator([1,2,3,4])
for (v: lst3) {
    v
}
"\n"

// Ranges

for (i: 1..5) i
"\n"

c = 0
for (c : -10,-8..0) c
"\n"

some_range = 1,3..10
for(i: some_range) {
    i
}
"\n"
for (j: some_range)
    j
"\n"

// For inside of constructor
class Particle {

    fun Particle(bounds) {
        this.velocity = []
        for (i: 0..2) {   
        }
        this.position = []
    }

    fun __String() {
        return "Pos: " ++ this.position ++ ", Vel: " ++ this.velocity
    }
}


p = Particle([[-50, 50], [-50, 50]])
p ++ "\n"
"done\n"

class WrongNext {
    fun __iter() {
        this.i = 0
        return this
    }

    fun __next(i) {
        raise StopIteration()
    }
}

try
    for (iwi : WrongNext()) {}
catch (e:TypeError)
    "caught\n"

@formatter
fun md(t) {
    return t[2..t.length()]
}

@converter("md", "txt")
fun md2txt(t) = t


note_str = md"# Some md header"
for (i : note_str) {
    i
}
"\n"
for (i : md"# Some more") {
    i
}
"\n"

l = [1,2,3]
for (a: l)
    for (b: l) {
        a++"-"++b++"|"
    }
"\n"