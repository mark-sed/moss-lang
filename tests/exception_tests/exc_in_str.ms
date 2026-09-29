class MyExc : Exception {
    fun MyExc(m:String) {
        ~super(m)
    }

    fun __String() {
        a
        return "MyExc"
    }
}

raise MyExc("fds")
