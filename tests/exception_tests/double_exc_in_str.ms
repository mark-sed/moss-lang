class MyExc2 {
    fun __String() {
        a
        return "MyExc2"
    }
}

class MyExc : Exception {
    fun MyExc(m:String) {
        ~super(m)
    }

    fun __String() {
        raise MyExc2()
        return "MyExc"
    }
}

raise MyExc("fds")
