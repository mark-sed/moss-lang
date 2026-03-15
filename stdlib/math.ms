d"""
Module for advanced mathematics and calculations.
"""

import ::Math.*

inf = Float("inf")
nan = Float("nan")

fun isinf(f:[Float,Int]) {
    d"Returns true if `f` is inf or -inf"
    return abs(f) == inf
}

fun isinf(c:Complex) {
    d"Returns true if `f` is inf or -inf"
    return ::isinf(c.real) || ::isinf(c.imag)
}

@internal
fun isnan(f:Float) {
    d"Returns true if `f` is nan or -nan."
}

fun isnan(i:Int) {
    d"Returns false as Int cannot be a nan."
    return false
}

fun isnan(c:Complex) {
    d"Return true if `c` (real or imaginary part) is a nan or -nan."
    return ::isnan(c.real) || ::isnan(c.imag)
}

fun abs(x:Complex) {
    d"""
    Returns an absolute value of `x`, which is calculated as:
        sqrt(a^2 + b^2) 
    """
    a = x.real
    b = x.imag
    return sqrt(a*a + b*b)
}

@internal
fun exp(x:[Int,Float]) {
    d"""
    Return e raised to the power `x`, where e is the base of natural logarithm.
    """
}

fun exp(z:Complex) {
    d"""
    Returns exponential of complex value `z`.
        e^(x+iy) = e^x(cos y + i * sin y)
    """
    x = z.real
    y = z.imag
    ex = ::exp(x)
    return Complex(ex * cos(y), ex * sin(y))
}

@internal
fun exp2(x:[Int,Float]) {
    d"""
    Return 2 raised to the power `x`.
    This should be more precise than doing `2^x`.
    """
}

@internal
fun cosh(x:[Int,Float]) {
    d"""
    Returns a hyperbolic cosine.
    """
}

@internal
fun sinh(x:[Int,Float]) {
    d"""
    Returns a hyperbolic sine.
    """
}

@internal
fun tanh(x:[Int,Float]) {
    d"""
    Returns a hyperbolic tangent.
    """
}

@internal
fun atan(x:[Int,Float]) {
    d"""
    Returns arctangent of `x`.
    """
}

@internal
fun atan2(x:[Int,Float], y:[Int,Float]) {
    d"""
    Returns arctangent of `y / x` in radians.
    """
}

fun sin(x:Complex) {
    r = ::sin(x.real) * cosh(x.imag)
    i = ::cos(x.real) * sinh(x.imag)
    return Complex(r, i)
}

fun cos(x:Complex) {
    r = ::cos(x.real) * cosh(x.imag)
    i = -::sin(x.real) * sinh(x.imag)
    return Complex(r, i)
}

fun tan(x:Complex) {
    xr = x.real
    xi = x.imag
    d = (cos(2*xr) + cosh(2*xi))
    r = sin(2*xr) / d
    i = sinh(2*xi) / d
    return Complex(r, i)
}

@internal
fun gcd(... ints) {
    d"""
    Returns Greatest Common Divisor of passed in Integers.
    If `ints` are not all Integers then TypeError is raised.
    If `ints` or all values are 0, then 0 is returned.
    """
}

@internal
fun lcm(... ints) {
    d"""
    Returns Least Common Multiple of passed in Integers.
    If `ints` are not all Integers then TypeError is raised.
    If `ints` or all values are 0, then 0 is returned.
    """
}