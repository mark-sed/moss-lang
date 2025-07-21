space MySpace {
    VAL1 = "val1\n"
    fun msf(a) {
        return a ++ "-msf\n"
    }
}

try {
    VAL1
} catch (e:NameError) {
    "caught\n"
}

import ::MySpace.msf

msf(1)

try {
    VAL1
} catch (e:NameError) {
    "caught\n"
}

import ::MySpace.*

MySpace.VAL1
MySpace.msf(2)
VAL1
msf(3)

fun local_space() {
    space MySpace {
        VAL1 = "local val1\n"
    }
    import ::(MySpace.VAL1)
    VAL1
    import ::(::MySpace.VAL1) as glob_val1
    glob_val1

    fun inner() {
        space MySpace {
            VAL1 = "inner local val1\n"
            VAL2 = "new val\n"
        }
        import ::MySpace.VAL1 as val1
        val1
        import ::$MySpace.VAL1 as non_loc_val1
        non_loc_val1 // non-local gets extended by local
        import ::(::MySpace.VAL1) as glob_val1_2
        glob_val1_2
    }

    ~inner()
    import ::(MySpace.VAL2)
    VAL2
}

~local_space()