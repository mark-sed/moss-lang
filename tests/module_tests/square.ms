import mod1

mod1.square(5)
"\n"

mod1.quiet_square(3) ++ "\n"
mod1.quiet_square(10) ++ "\n"
mod1.quiet_square(7) ++ "\n"

import mod1.*
square_all(4)
"\n"

// mod2 is imported from mod1
mod2.const_val(true)
"\n"
// mod2 can be also accessed from mod1
mod1.mod2.const_val(false)
"\n"

fun call_inner() {
    import mod3 as modik
    ~modik.outter_fun(1)
}

~call_inner()

import mod3.sum3
sum3(1,2,3)
"\n"

a = mod2.Sum(-1)
a.sum(5, 5)
"\n"

try {
    // Test that anonymous namespace values are not spilled with import
    mod1_val_an_sp
} catch (e:NameError) {
    "caught\n"
}

try {
    mod1.mod1_val_an_sp
} catch (e:NameError) {
    "caught\n"
}