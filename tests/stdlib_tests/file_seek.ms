f = open(args[0])
~f.seek(8)
f.read(9)
~f.seek_offset(-11)
f.read(11)
try
    ~f.seek(-100000)
catch (e:ValueError)
    "caught\n"
~f.close()