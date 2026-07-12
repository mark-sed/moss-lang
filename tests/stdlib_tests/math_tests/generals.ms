import math

// Test that moss.Math is spilled
math.cos(math.pi)
"\n"
math.sqrt(25)
"\n"

// Constants
math.inf
"\n"

math.nan
"\n"

// abs
math.abs(Complex(3, 4))        // 5.0
"\n"
math.abs(Complex(-3, -4))      // 5.0
"\n"
math.abs(Complex(0, 0))        // 0.0
"\n"
math.abs(Complex(5, 0))        // 5.0
"\n"
math.abs(Complex(0, -7))       // 7.0
"\n"
math.abs(Complex(1, 1))        // 1.4142135623730951
"\n"
math.abs(Complex(-2, 2))       // 2.8284271247461903
"\n"
math.abs(Complex(1e154, 1e154))  // 1.414213562373095e+154
"\n"
math.abs(Complex(0, 1e308))      // 1e+308
"\n"

math.isinf(Float("inf"))
"\n"
math.isinf(Float("-inf"))
"\n"
math.isinf(math.inf*math.inf)
"\n"
math.isinf(254.5)
"\n"
math.isinf(-122222.0)
"\n"

math.isnan(Float("nan"))
"\n"
math.isnan(math.inf / math.inf)
"\n"
math.isnan(Float("inf"))
"\n"
math.isnan(0.005)
"\n"

math.factorial(0);   // 1
"\n"
math.factorial(1);   // 1
"\n"
math.factorial(2);   // 2
"\n"
math.factorial(3);   // 6
"\n"
math.factorial(4);   // 24
"\n"
math.factorial(5);   // 120
"\n"
math.factorial(6);   // 720
"\n"
math.factorial(7);   // 5040
"\n"
math.factorial(8);   // 40320
"\n"
math.factorial(9);   // 362880
"\n"
math.factorial(10);  // 3628800
"\n"
math.factorial(11);  // 39916800
"\n"
math.factorial(12);  // 479001600
"\n"
math.factorial(13);  // 6227020800
"\n"
math.factorial(14);  // 87178291200
"\n"
math.factorial(15);  // 1307674368000
"\n"
math.factorial(16);  // 20922789888000
"\n"
math.factorial(17);  // 355687428096000
"\n"
math.factorial(18);  // 6402373705728000
"\n"
math.factorial(19);  // 121645100408832000
"\n"
math.factorial(20);  // 2432902008176640000
"\n"

try
    math.factorial(-1)
catch(e:ValueError)
    "caught\n"

try
    math.factorial(100)
catch(e:MathError)
    "caught\n"