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
