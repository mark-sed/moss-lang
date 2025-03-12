try {
    fwrg = File(".data_file_read.txt", "some_mode_lol")
    fwrg.open()
} catch(e:ValueError) {
    "caught\n"
}

try {
    fwrg = File("non_existent_file.lol12", "r")
    fwrg.open()
} catch(e:FileNotFoundError) {
    "caught\n"
}

f = open(".data_file_read.txt", "r")
f.__fstream
"\n"
f.readlines()
"\n"

path2 = ".created_file_t.txt1"
f2 = open(path2, "w")
~f2.write("written line\n")
~f2.close()

f3 = open(path2, "r")
f3.readlines()[0]
~f3.close()
"\n"