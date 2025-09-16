class B {}
b = B()

isinstance(b, B)
"\n"
isinstance(B, B)
"\n"

class A : B {}
a = A()
isinstance(a, A)
"\n"
isinstance(a, B)
"\n"
isinstance(b, A)
"\n"

isinstance(4, Int)
"\n"
isinstance(Math, String)
"\n"

enum En1 {
    A,
    B
}
try
    isinstance(En1.A, En1)
catch (e:TypeError) {
    "caught\n"
}

isinstance(4, [A, B])
"\n"
isinstance(a, [B, Int, A])
"\n"

try
    isinstance(4, [B, Int, 5])
catch (e:TypeError)
    "caught\n"