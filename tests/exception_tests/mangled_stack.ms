// This test "mangles" the stack frame by having different frames from 2
// modules.
// this calls mod_raiser, which does try catch all and calls back to this
// module (foo), foo then again calls inside of mod_raiser to __next, which
// raises. If the frames are not correctly inserted then the exception will be
// caught in mod_raiser.callit since it is in the same module and is in the
// stack.

import mod_raiser

fun foo() {
    o = mod_raiser.ran()
    o = o.__iter()
    try {
        while(true)
            o.__next()
    }
    catch (ef:StopIteration)
        "caught SI\n"
    finally {
        "finally in foo\n"
    }

    "End of foo\n"
    return 32
}

mod_raiser.callit(fun()=foo())
"\n"
"End\n"