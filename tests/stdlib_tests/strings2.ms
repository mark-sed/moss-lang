"|"
"hi".center(10)        // "    hi    "
"|\n"
"hi".center(10, "-")   // "----hi----"
"\n"
"hi".center(9, "-")    // "---hi----"
"\n"
"hello".center(3)      // "hello"
"\n"
"a".center(5, "*")     // "**a**"
"\n"
"abc".center(8, ".")   // "..abc..."
"\n"
"abc".center(9, ".")   // "...abc..."
"\n"
"".center(4, "-")      // "----"
"\n"
"x".center(1)          // "x"
"\n"
"x".center(2)          // "x "
"|\n"

"a\tb".expand_tabs()          // "a       b"
"\n"
"a\tb".expand_tabs(4)         // "a   b"
"\n"
"ab\tc".expand_tabs(4)        // "ab  c"
"\n"
"abcd\te".expand_tabs(4)      // "abcd    e"
"\n"
"ab\tcd\te".expand_tabs(4)    // "ab  cd  e"
"\n|"
"\t".expand_tabs(4)           // "    "
"|\n"
"a\t\tb".expand_tabs(4)       // "a       b"
"\n"
"a\tb\nc\td".expand_tabs(4)   // "a   b\nc   d"
"\n"
"".expand_tabs()              // ""
"no_tabs".expand_tabs(4)      // "no_tabs"
"\n"

"HelloWorld".remove_prefix("Hello")      // "World"
"\n"
"HelloWorld".remove_prefix("World")      // "HelloWorld"
"\n"
"abcabc".remove_prefix("abc")            // "abc"
"\n"
"abc".remove_prefix("abcd")              // "abc"
"\n"
"test".remove_prefix("")                 // "test"
"\n"
"prefix".remove_prefix("pre")            // "fix"
"\n"
"prefix".remove_prefix("prefix")         // ""
"prefix".remove_prefix("PREFIX")         // "prefix"
"\n"
"".remove_prefix("x")                    // ""

"HelloWorld".remove_suffix("World")     // "Hello"
"\n"
"HelloWorld".remove_suffix("Hello")     // "HelloWorld"
"\n"
"abcabc".remove_suffix("abc")           // "abc"
"\n"
"abc".remove_suffix("abcd")             // "abc"
"\n"
"test".remove_suffix("")                // "test"
"\n"
"filename.txt".remove_suffix(".txt")    // "filename"
"\n"
"filename.txt".remove_suffix(".pdf")    // "filename.txt"
"\n"
"CaseTest".remove_suffix("test")        // "CaseTest"
"\n"
"".remove_suffix("x")                   // ""

"hello".starts_with("he")     //  True
"\n"
"hello".starts_with("lo")     //  False
"\n"
"hello".starts_with("hello")  //  True
"\n"
"hello".starts_with("")       //  True
"\n"
"hello".starts_with("h")      // True
"\n"
"hello".starts_with("H")      // False
"\n"
"hello".starts_with("hello!") //  False
"\n"
"".starts_with("")            // True
"\n"
"".starts_with("a")           // False
"\n"
"123".starts_with("1")        // True
"\n"

"hello".ends_with("lo")      // True
"\n"
"hello".ends_with("he")      // False
"\n"
"hello".ends_with("hello")   // True
"\n"
"hello".ends_with("")        // True
"\n"
"hello".ends_with("o")       // True
"\n"
"hello".ends_with("O")       // False
"\n"
"hello".ends_with("world")   // False
"\n"
"".ends_with("")             // True
"\n"
"".ends_with("a")           // False
"\n"
"123".ends_with("3")        // True
"\n"

"hi".ljust(5)         // "hi   "
"|\n"
"hi".ljust(5, "-")    // "hi---"
"\n"
"hello".ljust(3)      // "hello"
"\n"
"a".ljust(4, "*")     // "a***"
"\n"
"abc".ljust(6, ".")   // "abc..."
"\n"
"abc".ljust(3)        // "abc"
"\n"
"".ljust(3, "-")      // "---"
"\n"
"x".ljust(1)          // "x"
"\n"
"x".ljust(2)          // "x "
"|\n"

"hi".rjust(5)
"|\n"
"hi".rjust(5, "-")
"\n"
"hello".rjust(3)
"\n"
"a".rjust(4, "*")
"\n"
"abc".rjust(6, ".")
"\n"
"abc".rjust(3)
"\n"
"".rjust(3, "-")
"\n"
"x".rjust(1)
"\n"
"x".rjust(2)
"|\n"

"   hello".lstrip()           // "hello"
"\n"
"\t\n hello programmer".lstrip()         // "hello programmer"
"\n"
"xxxyhello".lstrip("xy")      // "hello"
"\n"
"abcabcxyz".lstrip("abc")     // "xyz"
"\n"
"hello".lstrip("xyz")         // "hello"
"\n"
"".lstrip()                   // ""
"   ".lstrip()                // ""
"ababx".lstrip("ab")          // "x"
"\n"
"xhello".lstrip("x")          // "hello"
"\n"

"hello   ".rstrip()           // "hello"
"\n"
"hello\t\n".rstrip()          // "hello"
"\n"
"helloxxxy".rstrip("xy")      // "hello"
"\n"
"xyzabcabc".rstrip("abc")     // "xyz"
"\n"
"hello".rstrip("xyz")         // "hello"
"\n"
"".rstrip()                   // ""
"   ".rstrip()                // ""
"xabab".rstrip("ab")          // "x"
"\n"
"hellox".rstrip("x")          // "hello"
"\n"
"test".rstrip("")             // "test"
"\n"

"hello world".partition(" ")                // ["hello", " ", "world"]
"\n"
"abcabc".partition("bc")                    // ["a", "bc", "abc"]
"\n"
"hello".partition("x")                      // ["hello", "", ""]
"\n"
"key=value".partition("=")                  // ["key", "=", "value"]
"\n"
"no-separator".partition(":")               // ["no-separator", "", ""]
"\n"
"start-middle-end".partition("-")           // ["start", "-", "middle-end"]
"\n"
"aaa".partition("a")                        // ["", "a", "aa"]
"\n"
"aaa".partition("aa")                       // ["", "aa", "a"]
"\n"
"".partition("x")                           // ["", "", ""]
"\n"
"long string with spaces".partition(" ")    // ["long", " ", "string with spaces"]
"\n"

"hello world hello".rpartition(" ")        // ["hello world", " ", "hello"]
"\n"
"abcabc".rpartition("bc")                  // ["abca", "bc", ""]
"\n"
"hello".rpartition("x")                    // ["", "", "hello"]
"\n"
"key=value=123".rpartition("=")            // ["key=value", "=", "123"]
"\n"
"no-separator".rpartition(":")             // ["", "", "no-separator"]
"\n"
"start-middle-end".rpartition("-")         // ["start-middle", "-", "end"]
"\n"
"aaa".rpartition("a")                      // ["aa", "a", ""]
"\n"
"aaa".rpartition("aa")                     // ["a", "aa", ""]
"\n"
"".rpartition("x")                         // ["", "", ""]
"\n"
"long string with spaces".rpartition(" ")  // ["long string with", " ", "spaces"]
"\n"