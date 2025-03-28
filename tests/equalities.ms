[1,2,3] == [1,2,3]
"\n"
[1,2,3,4] == [1,2,3,5]
"\n"

Int == Int
"\n"
Int == Bool
"\n"
Bool == true
"\n"

class A {}

try 
    [A()] == [A()]
catch(e:TypeError)
    "caught\n"

class B {
    fun B(x) {
        this.x = x
    }

    fun (==)(b2:B) = this.x == b2.x
}

B(45) == B(7)
"\n"
B(45) == B(45)
"\n"

try
    B(8) == A()
catch(e:TypeError)
    "caught\n"

l1 = [1, 2, [B(5), B(6)], nil, []]
l2 = [1, 2, [B(6), B(7)], nil, []]
l3 = [1, 2, [B(5), B(6)], nil, []]

l1 == l2
"\n"
l1 == l1
"\n"
l1 == l3
"\n"

l3 != l1
"\n"
l3 != l2
"\n"

["ho", "hu"] == ["ho", "hu"]
"\n"

1 != nil
"\n"

print == print
"\n"

space SA{}

space SB{}

SA == SB
"\n"
SA != SA
"\n"
