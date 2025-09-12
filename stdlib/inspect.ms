d"""
Module for inspecting moss values.
"""

@internal
fun signature(f:Function) {
    d"""
    Returns Function `f` signature as a string.
    """
}

@internal
fun name(obj:[Type,Space,Function,FunctionList,Enum,Module]) {
    d"""
    Returns name of `obj`.
    """
}

@internal
fun enum_values(e:Enum) {
    d"""
    Returns list of values that enum `e` contains.
    """
}