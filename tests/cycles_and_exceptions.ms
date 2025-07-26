fun even(i) {
    if (i == 0)
        raise Exception("Got 0")
    return i % 2 == 0
}

for (i : 0..100) {
    try {
        if (i > 10) {
            "Value is too big to check!\n"
            break
        }
        if (even(i))
            continue
        i ++ " is odd\n"
    } catch (e) {
        ~print(e)
    } finally {
        ~print("Checked value: " ++ i)
    }
}

fun foo() {
    try {
        try 
            for (a: 0..2)
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

foo()
"\n"

try {
    for (i : 0..1) {
        try {
            break
        } catch (e) {
            "sfs\n"
        } finally {
            "finally 1\n"
        }
    }
    "LOOPED\n"
} catch (f) {
    "caught??\n"
} finally {
    "finally 2\n"
}
"DONE!\n"


for (i : Range(3)) {
    try {
        ~print(f"try: {i}")
        continue
    } catch (e) {
    } finally
        ~print(f"finally after continue: {i}")
}

for (i : Range(3)) {
    try {
        ~print(f"try: {i}")
        break
    } catch (e) {
    } finally
        ~print(f"finally after break: {i}")
}

for (i : Range(3)) {
    try {
        raise ValueError("oops")
    } catch (e) {
        ~print(f"catch {i}")
        break
    } finally
        ~print(f"finally after catch+break: {i}")
}

for (i : 0..3) {
    try {
        try {
            if (i == 2)
                break
            else if (i == 0)
                continue
        } catch (e) {
            "catch\n"
        } finally
            ~print(f"inner finally: {i}")
    } catch (e) {
        "catch 2\n"   
    } finally
        ~print(f"outer finally: {i}")
}

try {
    try {
        for (i : 1..10) {
            try {
                try {
                    f"{i}\n"
                    if (i == 3)
                        break
                    if (i == 1)
                        continue
                } catch (f) {

                } finally {
                    "loop finally inner\n"
                }
            } catch (e) {
            } finally {
                "loop finally outter\n"
            }
        }
        "done looping\n"
    } catch (e) {

    } finally {
        "finally inner\n"
    }
} catch (e) {

} finally {
    "finally global\n"
}


try {
    try {
        i = 0
        while (i < 10) {
            i += 1
            try {
                try {
                    f"{i}\n"
                    if (i == 3)
                        break
                    if (i == 1)
                        continue
                } catch (f) {

                } finally {
                    "loop finally inner\n"
                }
            } catch (e) {
            } finally {
                "loop finally outter\n"
            }
        }
        "done looping\n"
    } catch (e) {

    } finally {
        "finally inner\n"
    }
} catch (e) {

} finally {
    "finally global\n"
}

try {
    try {
        i = 0
        do {
            i += 1
            try {
                try {
                    f"{i}\n"
                    if (i == 3)
                        break
                    if (i == 1)
                        continue
                } catch (f) {

                } finally {
                    "loop finally inner\n"
                }
            } catch (e) {
            } finally {
                "loop finally outter\n"
            }
        } while (i < 10)
        "done looping\n"
    } catch (e) {

    } finally {
        "finally inner\n"
    }
} catch (e) {

} finally {
    "finally global\n"
}
