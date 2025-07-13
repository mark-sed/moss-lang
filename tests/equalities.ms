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

{:} == {:}
"\n"
{:} == {"A": 2}
"\n"
{:} != {:}
"\n"
{:} != {"A": 2}
"\n"

{"a": 3, "b": 4, "c": 5} == {"c": 5, "b": 4, "a": 3}
"\n"
a = {:}
a["A"] = 0
a["C"] = 0
a == {"C": 0, "A": 0}
"\n"

b = {1:"a"}
{1: [1,2,3], 2: {:}, 3: [{1:"a"}, nil], nil:true} == {3: [b, nil], 1: [1,2,3], 2: {:}, nil:true}
"\n"
{1: [1,2,3], 2: {:}, 3: [{1:"a"}, nil], nil:true} == {3: [b, nil], 1: [1,2,3], 2: {:}, nil:false}
"\n"
{1: [1,2,3,3], 2: {:}, 3: [{1:"a"}, nil], nil:true} == {3: [b, nil], 1: [1,2,3], 2: {:}, nil:true}
"\n"
b == b
"\n"