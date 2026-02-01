import math

math.gcd()
"\n"
math.gcd(42)
"\n"

math.gcd(0, 0)                 // 0
"\n"
math.gcd(0, 5)                 // 5
"\n"
math.gcd(5, 0)                 // 5
"\n"
math.gcd(18, 24)               // 6
"\n"
math.gcd(24, 18)               // 6
"\n"
math.gcd(-18, 24)              // 6
"\n"
math.gcd(18, -24)              // 6
"\n"
math.gcd(-18, -24)             // 6
"\n"
math.gcd(1, 9999991)           // 1
"\n"
math.gcd(48, 18, 30, 6)         // 6
"\n"
math.gcd(0, 0, 0)               // 0
"\n"
math.gcd(0, 0, 10)              // 10
"\n"

try {
    math.gcd(1, 2, 4.5)
} catch (e:TypeError)
    "caught\n"

math.lcm(0,0,0,0)
"\n"
math.lcm()
"\n"
math.lcm(-42)
"\n"

math.lcm(0, 0)                 // 0
"\n"
math.lcm(0, 5)                 // 0
"\n"
math.lcm(5, 0)                 // 0
"\n"
math.lcm(6, 8)                 // 24
"\n"
math.lcm(8, 6)                 // 24
"\n"
math.lcm(-6, 8)                // 24
"\n"
math.lcm(6, -8)                // 24
"\n"
math.lcm(-6, -8)               // 24
"\n"
math.lcm(4, 6, 10)             // 60
"\n"
math.lcm(0, 4, 6)              // 0
"\n"
math.lcm(1, 1, 1)              // 1
"\n"
math.lcm(7, 11, 13)            // 1001
"\n"

try
    math.lcm(0, nil)
catch (e:TypeError)
    "caught\n"
