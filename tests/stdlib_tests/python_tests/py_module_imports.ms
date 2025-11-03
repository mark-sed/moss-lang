import python

try {
    python.module("AAA_NON_Existent")
} catch (e: python.PythonException)
    "caught\n"

python.module("random")
"\n"

python.stdlib.get("int").call_moss(42)
"\n"

// Test initial population of stdlib
python.stdlib.bool.call_moss(42)
"\n"