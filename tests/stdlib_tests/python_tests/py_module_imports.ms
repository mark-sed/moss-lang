import python

try {
    python.module("AAA_NON_Existent")
} catch (e: python.PythonException)
    "caught\n"

python.module("random")
"\n"

python.builtins.get("int").call_moss(42)
"\n"

// Test initial population of builtins
python.builtins.bool.call_moss(42)
"\n"