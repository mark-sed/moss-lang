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

assert(sys.path.length() > 0, "No values in path")
assert("" in sys.path, "./ not in path")
~sys.path.append("/home/mosslibs")
assert("/home/mosslibs" in sys.path, "path not appended")