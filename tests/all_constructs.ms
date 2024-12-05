/**
 * Simple test for construct syntax
 */

a = 42
b = 9

// Ifs

if (a == b) a
else b

if (a > 8) 
{
    a ++ "\n"
}
else "small"

if (a > 4 and b < a) {
    condition = a > 9
    if (condition) {
        if(b < 4) "small\n"; else "big\n"
    }
}

b = a == 100 ? "ups" : 9

// Switch

switch(a) {
    case 0: print("not ok")
    case 1: {
        b += 1
        print("added 1")
    }
    case 2, 3, 4: print("wow")
    case 42: print("right answer")
    case nil: print("nil")
    default: print("oops")
}

// Whiles

while(true) {
    break
}

while (a > 0) a = -1

do {
    "in do\n"
} while(false)

do {
    a += 1
    a ++ " "
} while (a < 2)

"\n"

// For

for(i : 1,3..11) i

for (i : [1, 3, 11])
{
    i
}

for   (s : "text\n") i 

// Functions

fun foo(arg1, arg2) {
    return arg1 * arg2
}

foo(2, 4)

fun getID(x:Int) {
    return -1
}

fun getID(x) {
    return 42
}

getID(4)
getID("gi")

fun foo2(arg1) = arg1 * 2

(fun(arg1) = arg1 * 2)(6)

foo2(-7)

fun baz(a, b:Int, ... other, name="") {
}

// Spaces

space Math {
    fun abs(x:Int) {
        return x < 0 ? -x : x
    }
}

Math::abs(-5)

space {
    fun someCall() = 1

    aspc = someCall()
    aspc // A is printed and unaccessible
}

aspc // nil assignment

// Classes

class Range : Iterable, BaseClass {
    fun Range(start, end, step=1) {
        this.start = start
        this.end = end
        this.step = step
        this.i = start
    }

    fun __next() {
        if(this.step >= 0 and this.i >= this.end) return StopIteration
        if(this.step < 0 and this.i <= this.end) return StopIteration
        r = this.i
        this.i += this.step
        return r
    }
}

// Exceptions

class KnownExcp {
    fun nicemsg() = "Nice msg :)\n"
}

fun someCall() = raise KnownExcp()

try {
    someCall()
} catch (e:KnownExcp) {
    e.nicemsg()
} catch (e) {
    e
} finally {
    "This is always done"
}

// Annotations

@formatter
fun foo(s) = s

@converter("foo", "md")
fun foo2md(s) = s