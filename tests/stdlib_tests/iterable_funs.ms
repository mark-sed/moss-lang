length([1,2,3])
"\n"
length([])
"\n"
try
    length(34)
catch (e:TypeError)
    "caught\n"

class IterClass {
    fun IterClass(min, max) {
        this.min = min
        this.max = max
    }

    fun __iter() {
        this.i = this.min
        return this
    }

    fun __next() {
        if (this.i > this.max)
            raise StopIteration()
        v = this.i
        this.i += 1
        return v
    }
}

ic = IterClass(1, 5)
length(ic)
"\n"
length(ic)
"\n"
ic2 = IterClass(1, 0)
length(ic2)
"\n"