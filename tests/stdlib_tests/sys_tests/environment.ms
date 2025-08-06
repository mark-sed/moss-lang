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