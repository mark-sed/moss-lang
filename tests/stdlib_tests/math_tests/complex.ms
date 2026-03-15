import math.*

c = Complex(3, 4)
isinf(c)
"\n"
isinf(Complex(inf, 3))
"\n"
isinf(Complex(inf, -inf))
"\n"
isinf(Complex(-3, inf))
"\n"
isinf(Complex(inf, nan))
"\n"
isinf(Complex(nan, 3))
"\n"

isnan(Complex(inf, 3))
"\n"
isnan(Complex(inf, nan))
"\n"
isnan(Complex(nan, nan))
"\n"
isnan(Complex(4554.4, 300.3))
"\n"

exp(0)            // 1.0
"\n"
exp(1)            // 2.718281828459045
"\n"
exp(2)            // 7.38905609893065
"\n"
exp(0.0)          // 1.0
"\n"
exp(1.5)          // 4.4816890703380645
"\n"
exp(-1.0)         // 0.36787944117144233
"\n"
exp(Complex(0, 0))           // (1+0j)
"\n"
exp(Complex(0, 1))           // (0.5403023058681398+0.8414709848078965j)
"\n"
exp(Complex(1, 1))         // (1.4686939399158851+2.2873552871788423j)
"\n"
exp(Complex(-1, 2))        // (-0.1530918656742263+0.33451182923926226j)
"\n"
exp(Float("inf"))     // inf
"\n"
exp(Float("-inf"))    // 0.0
"\n"
exp(Float("nan"))     // nan
"\n"