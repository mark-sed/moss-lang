fun add_ex(v) = v++"!"

map(add_ex, [1,2,3,4,5])
"\n"

map(Int, [])
"\n"

class MyIter {
    fun MyIter(end) {
        this.end = end
    }

    fun __iter() {
        this.i = 0
        return this;
    }

    fun __next() {
        if (this.i >= this.end)
            raise StopIteration()
        v = this.i
        this.i += 1
        return v
    }

    fun ([])(index) {
        return index;
    }
}

map(lshift, MyIter(4), [1,1,1,1,1])
"\n"

map(lshift, MyIter(40), [])
"\n"

map(lshift, MyIter(4), [1,1])
"\n"

map(fun(x)=x*2, [1,2,3])
"\n"

map(fun(x, y)=x + y, [1, 2, 3], [10, 20, 30])
"\n"

map(fun(x, y)=x + y, [1, 2, 3], [10, 20])
"\n"

map(String.upper, ["abc", "def", "ghi"])
"\n"