v = print(1,2,3,4)
v = print("hi", "there", separator="-", end=".\n")
v = print()
v = print(true)
v = print(1,2,3,separator=",",end="!\n")

import sys
~print("Hello moss err!", file=sys.stderr, flush=true)

~print("Hello moss out!", file=sys.stdout, flush=true)

f = open(args[0], "w+")
~print("Print to a file", file=f)
f.read()
~f.close()

f2 = open(args[0], "a")
~print("Another print", file=f2)
~f2.close()

f3 = open(args[0], "a+")
f3.read()
~f3.close()