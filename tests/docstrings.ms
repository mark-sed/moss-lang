d"""
Module info
Second line
"""
__doc.strip()
"\n"

fun foo() {
    d"""
    Returns 42 """
    d"Second comment."
    return 42
}

foo.__doc.strip()
"\n"

class ACls {
    /*
    Comment
    */
    d"Class ACls"

    fun ACls() {
        // Also comment
        d"""
        Constructor
        """
    }
}

ACls.__doc.strip()
"\n"
ACls.ACls.__doc.strip()
"\n"

space SomeSp {
    d"""
    Some space
    """
}

SomeSp.__doc.strip()
"\n"
