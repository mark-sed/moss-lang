zip([1,2,3], [true, false, nil])
"\n"
zip([1,2,3,4,5], [])
"\n"
zip([1,2,3], [1,2,3,4], [-1, -2, -3])
"\n"
zip([1,2,3,4,5])
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

zip(MyIter(5), MyIter(3), MyIter(4))
"\n"
zip(MyIter(3), [true, false])
"\n"
