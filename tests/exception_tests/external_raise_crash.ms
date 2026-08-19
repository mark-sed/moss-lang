import mod_raiser

try {
    try {
        raise 4
    } catch (e:Int) {
        "caught 1\n"
        mod_raiser.exc()
    } finally {
        "finally 1\n"
    }
} catch (e) {
    "caught 2\n"
} finally {
    "finally 2\n"
}

try {
    raise 4
} catch (e:Int) {
    mod_raiser.exc()
} finally {
    "end\n"
}