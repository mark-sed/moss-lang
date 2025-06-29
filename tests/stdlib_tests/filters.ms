all([true, true])
"\n"
all([true, false])
"\n"
all([1,2,3,4], fun(x) = x < 5)
"\n"
all([1,2,3,4], fun(x) = x < 3)
"\n"

fun rec_all(x) {
    if (type(x) == List) {
        for (a: x) {
            if (not rec_all(a))
                return false
        }
        return true
    }
    return x
}
all([true, true, [true, [true, true]], true, true, [[true]]], rec_all)
"\n"
all([[true, true], [true, [false]], true, true], test=rec_all)
"\n"
all([])
"\n"

// Any
any([])
"\n"
any([true])
"\n"
any([false])
"\n"
any([true, true, false])
"\n"
any([false, false, false, false])
"\n"
any([1,2,3,4,5,6,"", 7], test=fun(x) = type(x) != Int)
"\n"
any([1,2,3,4,5,6,-7], test=fun(x) = type(x) != Int)
"\n"

// max
max([1,2.5,3,4])
"\n"
fun len(x) {
    return x.length()
}
max(["h", "hhh", "hello", "awesome"], key=len)
"\n"

try
    max([])
catch (e:ValueError)
    "caught\n"

max(["hi\n"])

max("123456789", key=Int)
"\n"

max([], def_val=nil)
"\n"

try
    min([])
catch (e:ValueError)
    "caught\n"

min(["hi\n"])

min("123456789", key=Int)
"\n"

min([], def_val=nil)
"\n"