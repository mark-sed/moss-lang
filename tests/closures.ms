// Accessing values of enclosing class
class OutterClass {

    NAME = "OC"

    class InnerClass {
        fun InnerClass() {
            // TODO: Unocomment OutterClass once closures are done? But also check if we want this
            "Created Inner + " ++ OutterClass.NAME ++ delimiter()
        }
    }

    fun delimiter() {
        return "; "
    }

    fun get_inner() {
        NAME ++ delimiter()
        return this.InnerClass()
    }
}

oc = OutterClass()
oc.get_inner()
"\n"
OutterClass.InnerClass++"\n"