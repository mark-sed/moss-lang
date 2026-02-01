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