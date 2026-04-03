fun raise_value(r) {
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

~raise_value("i")
~raise_value(nil)

// !!! We dont iterate in
try
    ~raise_value(2)
catch (e)
    "caught outter\n"
finally
    "finally outter\n"