import math

// atan
math.atan(0)        // 0.0
"\n"
math.atan(1)        // 0.7853981633974483   (π / 4)
"\n"
math.atan(-1)       // -0.7853981633974483  (-π / 4)

"\n"
math.atan(0.5)      // 0.4636476090008061
"\n"
math.atan(2)        // 1.1071487177940904

"\n"
math.atan(10)       // 1.4711276743037347
"\n"
math.atan(-10)      // -1.4711276743037347

"\n"
math.atan(Float("inf")) == math.pi / 2   // (π / 2)
"\n"
math.atan(Float("-inf")) == -math.pi / 2 // (-π / 2)

"\n"
math.atan(1e-10) == 1e-10   // 1e-10 (approximately)
"\n"

// atan2
math.atan2(0, 1)        // 0.0
"\n"
math.atan2(1, 0)        // 1.5707963267948966   (π / 2)
"\n"
math.atan2(0, -1)       // 3.141592653589793    (π)
"\n"
math.atan2(-1, 0)       // -1.5707963267948966  (-π / 2)

"\n"
math.atan2(1, 1)        // 0.7853981633974483   (π / 4)
"\n"
math.atan2(-1, 1)       // -0.7853981633974483  (-π / 4)
"\n"
math.atan2(1, -1)       // 2.356194490192345    (3π / 4)
"\n"
math.atan2(-1, -1)      // -2.356194490192345   (-3π / 4)

"\n"
math.atan2(2, 3)        // 0.5880026035475675
"\n"
math.atan2(0, 0)        // 0.0
"\n"
math.atan2(1, math.inf) // 0.0
"\n"
math.atan2(1, -math.inf) == math.pi // 3.141592653589793    (π)
"\n"