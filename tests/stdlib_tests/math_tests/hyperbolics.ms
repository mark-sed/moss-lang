import math.*

cosh(20)       // 242582597.70489514
"\n"
cosh(-20)      // 242582597.70489514
"\n"
cosh(71000.0)    // inf        (overflow to infinity on most platforms)
"\n"

sinh(20)       // 242582597.70489514
"\n"
sinh(-20)      // -242582597.70489514
"\n"
sinh(71000.0)    // inf        (positive overflow)
"\n"
sinh(-71000.0)   // -inf       (negative overflow)
"\n"

tanh(20)       // 1.0        (approaches +1)
"\n"
tanh(-20)      // -1.0       (approaches -1)
"\n"
tanh(710.0)    // 1.0
"\n"
tanh(-710.0)   // -1.0
"\n"

cosh(inf)      // inf
"\n"
cosh(-inf)     // inf
"\n"
cosh(nan)      // nan
"\n"

sinh(inf)      // inf
"\n"
sinh(-inf)     // -inf
"\n"
sinh(nan)      // nan
"\n"

tanh(inf)      // 1.0
"\n"
tanh(-inf)     // -1.0
"\n"
tanh(nan)      // nan
"\n"

tanh(0.0 * inf)  // nan
"\n"
sinh(0.0 * inf)  // nan
"\n"
cosh(0.0 * inf)  // nan
"\n"