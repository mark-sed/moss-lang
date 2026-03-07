(0.5).as_int_ratio()    // (1, 2)
"\n"
(0.25).as_int_ratio()   // (1, 4)
"\n"
(1.0).as_int_ratio()    // (1, 1)
"\n"
(2.0).as_int_ratio()    // (2, 1)
"\n"

(0.75).as_int_ratio()   // (3, 4)
"\n"
(1.5).as_int_ratio()    // (3, 2)
"\n"

(0.1).as_int_ratio()    // (3602879701896397, 36028797018963968)
"\n"

(-0.5).as_int_ratio()   // (-1, 2)
"\n"
(-1.25).as_int_ratio()  // (-5, 4)
"\n"

(0.0).as_int_ratio()    // (0, 1)
"\n"

try
    Float("inf").as_int_ratio()   //
catch (e:MathError)
    "caught\n"

try
    Float("-inf").as_int_ratio()  //
catch (e:MathError)
    "caught\n" 

try
    Float("nan").as_int_ratio()   // 
catch (e:MathError)
    "caught\n"