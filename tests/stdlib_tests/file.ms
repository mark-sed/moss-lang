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

class LoremIpsum {

    fun LoremIpsum(am:Int) {
        this.am = am
    }

    fun __String() {
        t = ""
        for (i : 0..this.am) {
            t ++= "Lorem Ipsum\n"
        }
        return t
    }
}

f4 = open(".created_file_2.txt", "w")
~f4.write(LoremIpsum(2))
~f4.close()

f5 = with_open(".created_file_2.txt", fun(f)=f.readlines(),"r")
for (l: f5)
    l ++ "\n"

with_open(".created_file_2.txt", fun(f)=f.read(6))
with_open(".created_file_2.txt", fun(f)=f.read())

"---\n"
f6 = open(".created_file_2.txt")
f6.readln(6)
"|"
try {
    while (true)
        f6.readln()
} catch (e: EOFError)
    "end\n"
~f6.close()

with_open(".created_file_2.txt", fun(x)=x.read(5), "rb")
"\n"
with_open(".created_file_2.txt", fun(x)=x.read(), "rb")
"\n"
cnt_b = with_open(".created_file_2.txt", fun(x)=x.read(1000), "rb")
for (b: cnt_b) {
    chr(b)
}

f = open(".created_binary_f.bytes", "wb")
~f.write(Bytes("\xFF\xFFABC"))
~f.close()

with_open(".created_binary_f.bytes", fun(x)=x.read(), "rb")
"\n"

class MyBytes : Bytes {
    fun MyBytes(s) {
        ~super(s++"\x00")
    }
}

f = open(".created_binary_f.bytes", "wb")
try {
    f.write(42)
} catch (e:TypeError)
    "caught\n"

~f.write(MyBytes("\XFF\XAA"))
~f.close()

try 
    f.readlines()
catch (e:ValueError) {
    "caught\n"
}

try 
    f.write("hi")
catch (e:ValueError) {
    "caught\n"
}

try 
    f.read()
catch (e:ValueError) {
    "caught\n"
}

try 
    f.readln()
catch (e:ValueError) {
    "caught\n"
}

with_open(".created_binary_f.bytes", fun(f)=f.read(), "rb")
"\n"
