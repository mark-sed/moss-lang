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

(2^63 - 1).bit_count()   // 63
"\n"
(-(2^63 - 1)).bit_count()// 63
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

(2^63 - 1).bit_length()    // 63
"\n"
(-(2^63 - 1)).bit_length() // 63
"\n"
