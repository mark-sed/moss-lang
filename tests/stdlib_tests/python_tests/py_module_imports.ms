import python

try {
    python.module("AAA_NON_Existent")
} catch (e: python.PythonException)
    "caught\n"

python.module("random")
"\n"