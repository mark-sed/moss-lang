fun callit(f) {
    try {
        f()
        "try callit done!\n"
    } catch (e) {
        "caught\n"
        raise e
    } finally {
        "finally in callit\n"
    }
    "Callit end\n"
    return true
}


class ran {
    fun ran() {
        this.i = 0
    }

    fun __next() {
        "__next\n"
        if (this.i >= 1)
            raise StopIteration()
        this.i += 1
        return this.i
    }

    fun __iter() {
        return this
    }
}

fun exc() {
    class SomeVal {
        fun SomeVal(a) {
            this.a = a
        }
        fun __String() = "SomeVal " ++ this.a ++ "\n"
    }
    raise SomeVal(22)
}