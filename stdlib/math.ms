d"""
Module for advanced mathematics and calculations.
"""

import ::Math.*

inf = Float("inf")
nan = Float("nan")

fun abs(x:Complex) {
    a = x.real
    b = x.imag
    return sqrt(a*a + b*b)
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