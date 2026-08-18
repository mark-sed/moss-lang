d"""
Module for working with built-in Moss warnings.
These warnings behave just like standard Moss warnings, meaning that they
abide by the `-W` option. So unless `-W all` or `-W error` is set, nothing
will be printed.
"""

@internal
fun warn(msg:String) {}