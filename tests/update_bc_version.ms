f = open(args[0], "r+b")
version = Int(args[1])

~f.seek(8)
~f.write(version.to_bytes(1))
~f.close()
