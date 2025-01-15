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