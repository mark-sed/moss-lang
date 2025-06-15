fun unpacker(... args) {
    ~print(<<args)
}

~unpacker(1, true, "hi")

fun const_unpack() {
    ~print(<<[1,2,3])
}

~const_unpack()

fun unpack_local(a) {
    lst = [a, true, a]
    ~print(<<lst)
}

lst = [6, true, 7]
~print(<<lst)

~unpack_local(lst)

lst2 = [lst, lst]
~print(<<lst2[0][-1..-5])

msg = "ahoj"
~print(<<msg)

~print(<<"lidi")

try
    ~print(<<42)
catch(e:TypeError)
    "caught\n"

try
    ~print(<<print)
catch(e:TypeError)
    "caught\n"

class MyList {
    fun MyList(... v) {
        this.vals = [0] + v + [0] 
    }

    fun __iter() {
        return this.vals
    }
}

ml = MyList(1,2,3)
~print(<<ml)

class MyList2 {
    fun MyList2(... v) {
        this.vals = ["|"] + v + ["|"] 
    }

    fun __iter() {
        this.i = 0
        return this
    }

    fun __next() {
        if (this.i >= this.vals.length()) {
            raise StopIteration()
        }
        v = this.vals[this.i]
        this.i += 1
        return v
    }
}

ml2 = MyList2("a", "b", "c")
~print(<<ml2, 2)

class MyListIterableOnce {
    fun MyListIterableOnce(... v) {
        this.vals = ["|"] + v + ["|"] 
        this.i = 0
    }

    fun __next() {
        if (this.i >= this.vals.length()) {
            raise StopIteration()
        }
        v = this.vals[this.i]
        this.i += 1
        return v
    }
}

ml3 = MyListIterableOnce(0,1,2)
~print(<<ml3)
~print(<<ml3) // \n

class ClsX {}
clsx = ClsX()
try
    ~print(<<clsx)
catch(e:TypeError)
    "caught\n"

// Dict unpacking

fun foov(a, b, c) {
    return a ++ b ++ c ++ "\n"
}

argdict = {"c": "C", "a": "A", "b": "B"}
foov(<<argdict)

argdict2 = {"a": 1}
foov(<<argdict2, c=3, b=2)

fun foov2() {
    return "called\n"
}

foov2(<<{:})

fun foov3(a:Int) {
    return "foov3\n"
}

try
    foov3(<<{
        "a": true
    })
catch (e: TypeError) {
    "caught\n"
}

try
    foov3(<<{:})
catch (e: TypeError) {
    "caught\n"
}

foov3(<<{:}, 2)

fun foov4(a, b="B", c:String="C") {
    return a++b++c++"\n"
}

foov4(<<{"a": "a"})
foov4(b="b", <<{"a": "A"}, c="c")
foov4(4, <<{:})