cpp.clong(42000000).to_moss()
"\n"
cpp.cdouble(-42000000.54321).to_moss()
"\n"
cpp.cchar_star("Hello").to_moss()
"\n"
cpp.cbool(false).to_moss()
"\n"
cpp.cfloat(-0.54).to_moss()
"\n"
cpp.cfloat(16777217.0).to_moss() // wont fit into float
"\n"
cpp.cint(-451).to_moss()
"\n"
cpp.cint(56666666).to_moss()
"\n"
cpp.cint(2147483648).to_moss() // wont fit into int
"\n"

cpp.cint8_t(100).to_moss()
"\n"
cpp.cint8_t(-101).to_moss()
"\n"
cpp.cint8_t(0).to_moss()
"\n"
cpp.cint8_t(130).to_moss()
"\n"

cpp.cint16_t(32767).to_moss()       // 32767
"\n"
cpp.cint16_t(32768).to_moss()       // -32768
"\n"
cpp.cint16_t(-32769).to_moss()      // 32767
"\n"
cpp.cint16_t(65535).to_moss()       // -1
"\n"

cpp.cint32_t(2147483647).to_moss()      // 2147483647
"\n"
cpp.cint32_t(2147483648).to_moss()    // -2147483648
"\n"
cpp.cint32_t(-2147483649).to_moss()   // 2147483647
"\n"
cpp.cint32_t(4294967295).to_moss()    // -1
"\n"

cpp.cint64_t(9223372036854775807).to_moss()   // 9223372036854775807
"\n"
cpp.cint64_t(-9223372036854775807).to_moss()  // -9223372036854775807
"\n"
cpp.cint64_t(0).to_moss()                      // 0
"\n"
cpp.cint64_t(-1).to_moss()                     // -1
"\n"

cpp.cuint(4294967295).to_moss()               // 4294967295
"\n"
cpp.cuint(0).to_moss()                         // 0
"\n"
cpp.cuint(1).to_moss()                         // 1
"\n"

cpp.culong(4294967295).to_moss()               // 4294967295
"\n"
cpp.culong(0).to_moss()                        // 0
"\n"
cpp.culong(1).to_moss()                        // 1
"\n"

cpp.cuint8_t(255).to_moss()                    // 255
"\n"
cpp.cuint8_t(0).to_moss()                      // 0
"\n"
cpp.cuint8_t(1).to_moss()                      // 1
"\n"

cpp.cuint16_t(65535).to_moss()                 // 65535
"\n"
cpp.cuint16_t(0).to_moss()                     // 0
"\n"
cpp.cuint16_t(1).to_moss()                     // 1
"\n"

cpp.cuint32_t(4294967295).to_moss()             // 4294967295
"\n"
cpp.cuint32_t(0).to_moss()                     // 0
"\n"
cpp.cuint32_t(1).to_moss()                     // 1
"\n"

cpp.cuint64_t(4294967295).to_moss() // 4294967295
"\n"
cpp.cuint64_t(0).to_moss()                     // 0
"\n"
cpp.cuint64_t(1).to_moss()                     // 1
"\n"

cpp.cuint8_t(-1).to_moss()     // 255
"\n"
cpp.cuint16_t(-1).to_moss()    // 65535
"\n"
cpp.cuint32_t(-1).to_moss()    // 4294967295
"\n"

cpp.cushort(65535).to_moss()                 // 65535
"\n"
cpp.cushort(0).to_moss()                     // 0
"\n"
cpp.cushort(1).to_moss()                     // 1
"\n"

cpp.cshort(32767).to_moss()       // 32767
"\n"
cpp.cshort(32768).to_moss()       // -32768
"\n"
cpp.cshort(-32769).to_moss()      // 32767
"\n"
cpp.cshort(65535).to_moss()       // -1
"\n"

cpp.cchar("A").to_moss()
"\n"
cpp.cchar(65).to_moss()
"\n"
cpp.cchar(42).to_moss()
"\n"
cpp.cchar(42).to_moss_int()
"\n"
cpp.cchar(127).to_moss_int()
"\n"

cpp.cuchar("A").to_moss()
"\n"
cpp.cuchar(65).to_moss()
"\n"
cpp.cuchar(42).to_moss()
"\n"
cpp.cuchar(42).to_moss_int()
"\n"
cpp.cuchar(250).to_moss_int()
"\n"

try
    cpp.cchar("")
catch (e:ValueError)
    "caught\n"

try
    cpp.cchar("hello")
catch (e:ValueError)
    "caught\n"

try
    cpp.cuchar("42")
catch (e:ValueError)
    "caught\n"