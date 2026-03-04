0 .bit_count()            // 0
"\n"
1 .bit_count()            // 1
"\n"
2 .bit_count()            // 1
"\n"
3 .bit_count()            // 2
"\n"
13 .bit_count()           // 3   (1101)
"\n"
255 .bit_count()          // 8   (11111111)
"\n"

(-1).bit_count()          // 1
"\n"
(-13).bit_count()         // 3
"\n"

(2^10).bit_count()       // 1
"\n"
(2^10 - 1).bit_count()   // 10
"\n"

(2^62 - 1).bit_count()   // 62
"\n"
(-(2^62 - 1)).bit_count()// 62
"\n"

0 .bit_length()             // 0
"\n"
1 .bit_length()             // 1
"\n"
2 .bit_length()             // 2
"\n"
3 .bit_length()             // 2
"\n"
4 .bit_length()             // 3
"\n"
7 .bit_length()             // 3
"\n"
8 .bit_length()             // 4
"\n"
13 .bit_length()            // 4
"\n"

(-1).bit_length()           // 1
"\n"
(-13).bit_length()          // 4
"\n"


(2^10).bit_length()        // 11
"\n"
(2^10 - 1).bit_length()    // 10
"\n"

(2^62 - 1).bit_length()    // 62
"\n"
(-(2^62 - 1)).bit_length() // 62
"\n"

(0).to_bytes(1, "big")                 // b'\x00'
"\n"
(1).to_bytes(1, "big")                 // b'\x01'
"\n"
(255).to_bytes(1, "big")               // b'\xff'
"\n"

(256).to_bytes(2, "big")               // b'\x01\x00'
"\n"
(256).to_bytes(2, "little")            // b'\x00\x01'
"\n"

(1025).to_bytes(2, "big")              // b'\x04\x01'
"\n"
(1025).to_bytes(2, "little")           // b'\x01\x04'
"\n"

(-1).to_bytes(1, "big", signed=true)   // b'\xff'
"\n"
(-1).to_bytes(2, "big", signed=true)   // b'\xff\xff'
"\n"

(-128).to_bytes(1, "big", signed=true) // b'\x80'
"\n"
(-129).to_bytes(2, "big", signed=true) // b'\xff\x7f'
"\n"

(127).to_bytes(1, "big", signed=true)  // b'\x7f'
"\n"
(128).to_bytes(2, "big", signed=true)  // b'\x00\x80'
"\n"

(2^62 - 1).to_bytes(8, "big")         
"\n"
(-(2^   62)).to_bytes(8, "big", signed=true) 
"\n"

try
    (-1).to_bytes(1, "big")                 // MathError (negative with signed=False)
catch(e:MathError)
    "caught\n"

try
    (256).to_bytes(1, "big")                // MathError (does not fit in 1 byte)
catch(e:MathError)
    "caught\n"

try
    (128).to_bytes(1, "big", signed=true)   // MathError (too large for signed 1 byte)
catch(e:MathError)
    "caught\n"

try
    (-129).to_bytes(1, "big", signed=true)  // MathError (too small for signed 1 byte)
catch(e:MathError)
    "caught\n"

try
    (1).to_bytes(0, "big")                  // MathError (length too small)
catch(e:MathError)
    "caught\n"

try
    (-1).to_bytes(0, "big", signed=true)    // MathError (length too small)
catch(e:MathError)
    "caught\n"

try
    (1).to_bytes(1, "middle")               // ValueError (invalid byteorder)
catch(e:ValueError)
    "caught\n"