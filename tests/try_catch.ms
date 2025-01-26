fun foo_raise(ex) {
    try {
        raise ex
    } catch (e2:[NameError, Int, Bool]) {
        "Caught "++e2++"!\n"
    } catch (f2) {
        "other\n"
    }
}

try {
    "Success\n"
} catch (e1:[NameError, Int]) {
    "Caught "++e1++"!\n"
} catch (f1) {
    "in f\n"
} finally {
    "finally end\n"
}

try {
    raise NameError("Unkown name")
} catch (e2:[NameError, Int]) {
    "Caught "++e2++"!\n"
} catch (f2) {
    "in f\n"
} finally {
    "finally end\n"
}

~foo_raise(true)
~foo_raise(nil)

try {
    raise 3
} catch (e3:[NameError, Int]) {
    "Caught "++e3++"!\n"
} catch (f3) {
    "in f\n"
} finally {
    "finally end\n"
}

try {
    raise "str"
} catch (e4:[NameError, Int]) {
    "Caught "++e4++"!\n"
} catch (f4) {
    "in f\n"
} finally {
    "finally end\n"
}

import ext_module.mod_raise

try {
    mod_raise(8)
} catch (d) {
    "Caught: " ++ d ++ "\n"
}

"module end\n"