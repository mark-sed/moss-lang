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