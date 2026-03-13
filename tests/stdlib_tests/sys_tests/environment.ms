import sys

// Lets just see it it does not fail.
// TODO: First set some and then look it up here.
//       PATH is not guaranteed to exist.
~sys.getenv("PATH")
try
    sys.getenv("totallynon-existent-envvalue123")
catch (e:KeyError)
    "caught\n"

sys.getenv("totallynon-existent-envvalue123", "does not exist\n")

// These tests messup imports add other tests before it!

assert(sys.get_path().length() > 0, "No values in path")
assert("" in sys.get_path(), "./ not in path")
~sys.add_path("/home/mosslibs")
assert("/home/mosslibs" in sys.get_path(), "path not appended")

~sys.set_path("", "/usr-new/lib")
sys.get_path()
"\n"
~sys.add_path("start", prepend=true)
~sys.add_path("end", prepend=false)
sys.get_path()
"\n"

~sys.set_path()
try
    import empty
catch (e:ModuleNotFoundError)
    "caught\n"

~sys.add_path(args[0])
import empty
empty
"\n"