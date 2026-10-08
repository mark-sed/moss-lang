import sys

~sys.stderr.write("Erroutput\n")
~sys.stderr.close()
try
    sys.stderr.write("bbbb\n")
catch (e:ValueError)
    "caught\n"

~sys.stdout.write("Out!\n")

try
    sys.stdout.read()
catch (e:ValueError)
    "caught\n"

sys.stdout.mode
"\n"