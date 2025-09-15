fun foo(def_val=nil) {
    try {
        raise 1
    } catch (e) {
        "Caught\n"
        return e
    } finally {
        "Ran finally\n"
        if (def_val != nil)
            return def_val
    }
}

foo()
"\n"
foo(42)
"\n"

fun foo2(def_val=nil) {
    try {
        return "hi\n"
        "Sdafs\n"
    } catch (e) {

    } finally {
        "Ran finally\n"
        if (def_val != nil)
            return def_val
    }
}

foo2()
foo2(8)
"\n"

try {
    "in try\n"
} catch (e) {
    "in catch??\n"
} finally {
    "in finally\n"
}

fun return_in_try(p, func) {
    "Start\n"
    try {
        return func(p)
    } catch(v) {
        "caught\n"
    } finally {
        "CLOSING\n"
    }
}

return_in_try("hi\n", fun(f)=f)
"\n"

fun foo(f) {
    raise 34
}

return_in_try("nope", foo)

"\n"

fun foo_int(x, v) {
    try {
        return x / v
    } catch (e) {
        return Float("inf")
    } finally {
        "Running division\n"
    }
}

foo_int(5, 2)
"\n"
foo_int(5, 0)
"\n"

try {
    try {
        "in try\n"
    } catch (e) {
    } finally {
        raise 42
        "in finally\n"
    }
} catch (v) {
    "caught: " ++ v ++ "\n"
} finally {
    "ran finally\n"
}

try {
    raise 3
} catch (e) {
    "caught: " ++ e ++ "\n"
} finally {
    try {
        raise true
    } catch (e) {
        "caught: " ++ e ++ "\n" 
    } finally {
        try {
            raise -1
        } catch (e:Int) {
            "caught: " ++ e ++ "\n"
        } finally {
            "finally 3 done\n"
        }
        "finally 2 done\n"
    }
    "finally 1 done\n"
}

fun foo34() {
    try {
        try 
            return 2
        catch (e)
            "sf\n"
        finally {
            "finally 1\n"
        }
    } catch (f) {
        "caught??\n"
    } finally {
        "finally 2\n"
    }
    "DONE!\n"
}

foo34()
"\n"


fun f1() {
    try {
        raise 4
    } catch(v) {
        "catch\n"
        raise v
    } finally {
        "Finally\n"
    }
}

try  {
    try
        f1()
    catch(g:Int) {
        "caught 3\n"
        raise g
    }
    finally
        "finally 3\n"
}
catch (e)
    "catch2\n"
finally
    "finally2\n"

// TODO: Uncomment once fixed
/*
try  {
    try
        f1()
    catch(g:SyntaxError)
        "caught 3\n"
    finally
        "finally 3\n" // This is not outputted
}
catch (e)
    "catch2\n"
finally
    "finally2\n"
*/