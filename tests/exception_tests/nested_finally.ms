fun raise_value(r, early_exit:Bool=false) {
    if (early_exit) {
        "> exit\n"
        return
    }
    ">"
    try {
        try {
            try {
                try {
                    raise r
                } catch (s:String) {
                    "catch string\n"
                } catch (b:Bool) {
                    "catch bool\n"
                } finally {
                    "finally 1\n"
                }
            } catch (f:Float) {
                "caught float\n"
            } finally {
                "finally 2\n"
            }
        } catch (n:NilType) {
            "caught nil\n"
        } finally {
            "finally 3\n"
        }
    } catch (e:Exception) {
        "catch exception\n"
    } finally {
        "finally 4\n"
    }
}

fun no_raise() {
    try {
        try {
            "> "
        } catch (e:String) {
            "caught String???!\n"
        } catch (f) {
            "caught anything??!\n"
        } finally {
            "finally in no raise 1\n"
        }
    } catch (t) {
        "caught t???!\n"
    } finally {
        "finally in no raise 2\n"
    }
}

~raise_value("i")
~raise_value(nil)

~raise_value("i", early_exit=true)

~no_raise()

try
    ~raise_value(2)
catch (e)
    "caught outter\n"
finally
    "finally outter\n"
