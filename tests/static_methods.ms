class StatClass {
    fun StatClass(x) {
        this.x = x
    }

    @staticmethod
    fun get_info(name) {
        try {
            return name ++ ": " ++ this.x ++ "\n"
        } catch(e:NameError) {
            return "caught\n"
        }
    }

    @staticmethod
    fun get_info2(name, id:Int) {
        return name ++ ", " ++ id ++ "\n"
    }
}

sc = StatClass(42)
sc.get_info("x")
sc.get_info2("id", 6)
try {
    StatClass.get_info2("id", 6, sc)
} catch(e:TypeError) {
    "caught\n"
}