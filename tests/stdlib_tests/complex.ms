c1 = Complex(1, -2)
c1
"\n"
c1.real ++ "," ++ c1.imag ++ "\n"

c1 == c1
"\n"

c2 = Complex(5)
c2
"\n"

c1 == c2
"\n"

Complex(1) != Complex(1)
"\n"
Complex(2, 0.5) != Complex(2, 1)
"\n"

c3 = Complex(-1.5, 3.2)
c3
"\n"

c1 + c2
"\n"
c1 + c3
"\n"
c1 - c2
"\n"
c1 * c2
"\n"
Complex(2, 4) * Complex(6, -3) == Complex(6, -3) * Complex(2, 4)
"\n"
Complex(10, -3) / Complex(2, 4)
"\n"
try {
    c1 / Complex()
} catch (e:DivisionByZeroError) {
    "caught\n"
}
-c1
"\n"

c2 == 5.0
"\n"
c1 == 1
"\n"
c2 != 5.0
"\n"
c2 != 30
"\n"

c2 - 5
"\n"
c1 - 1
"\n"

c2 + 10.0
"\n"
Complex() + 8
"\n"

Complex(2, 3) * 9
"\n"
c2 * 0
"\n"

Complex(10, -3) / 2
"\n"
try {
    Complex(8) / 0
} catch (e:DivisionByZeroError) {
    "caught\n"
}

~print(Complex(1, 2) + Complex(3, 4))     // (4.0+6.0i)
~print(Complex(5, -1) - Complex(2, 3))    // (3.0-4.0i)
~print(Complex(2, 3) * Complex(4, -5))    // (23.0+2.0i)
~print(Complex(7, 4) / Complex(3, -2))    // (1.0+2.0i)
~print(Complex(3, 0) + 5)                 // (8.0+0.0i)
~print(-Complex(2, 1) + 5)                // (3.0-1.0i)
~print(Complex(1, -1) * 2)                // (2.0-2.0i)
~print(Complex(6, 8) / 2)                 // (3.0+4.0i)