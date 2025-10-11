lst = [0,1,2,3,4,5]
~lst.delete(1..4)
lst
"\n"

~lst.delete(0)
lst
"\n"

lst2 = [0,1,2,3,4,5,6]
~lst2.delete(10..2)
lst2
"\n"

lst3 = [0,1,2,3,4,5,6,7,8]
~lst3.delete((0,2..100))
lst3
"\n"
~lst3.delete(-10..10)
lst3
"\n"

lst4 = [0,1,2,3,4,5,6]
~lst4.delete((100..-2))
lst4
"\n"

lst5 = [0,1,2,3,4,5]
~lst5.delete((10..15))
lst5
"\n"

try
    lst5.delete(10)
catch(e:IndexError)
    "caught\n"

for (i: 0..length(lst5)) {
    ~lst5.delete(0)
    lst5
    "\n";
}

lst6 = [0,1,2,3,nil,4]
~lst6.remove(nil)
lst6
"\n"

try
    lst6.remove(true)
catch (e: ValueError)
    "caught\n"

lst7 = [0, "None", 1, 2, "None", "None", 3, "None"]
while (true) {
    try
        ~lst7.remove("None")
    catch (e:ValueError)
        break
}
lst7
"\n"