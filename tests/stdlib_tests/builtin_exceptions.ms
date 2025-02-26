try {
    raise Exception("")
} catch (e:Exception) {
    "Caught: "++e++"\n"
}

try {
    raise NameError("")
} catch (e:NameError) {
    "Caught: "++e++"\n"
}

try {
    raise AttributeError("")
} catch (e:AttributeError) {
    "Caught: "++e++"\n"
}

try {
    raise ModuleNotFoundError("")
} catch (e:ModuleNotFoundError) {
    "Caught: "++e++"\n"
}

try {
    raise TypeError("")
} catch (e:TypeError) {
    "Caught: "++e++"\n"
}

try {
    raise AssertionError("")
} catch (e:AssertionError) {
    "Caught: "++e
}

try {
    raise NotImplementedError("")
} catch (e:NotImplementedError) {
    "Caught: "++e++"\n"
}

try {
    raise ParserError("")
} catch (e:ParserError) {
    "Caught: "++e++"\n"
}

try {
    raise SyntaxError("")
} catch (e:SyntaxError) {
    "Caught: "++e++"\n"
}

try {
    raise LookupError("")
} catch (e:LookupError) {
    "Caught: "++e++"\n"
}

try {
    raise IndexError("")
} catch (e:IndexError) {
    "Caught: "++e++"\n"
}

try {
    raise MathError("")
} catch (e:MathError) {
    "Caught: "++e++"\n"
}

try {
    raise DivisionByZeroError("")
} catch (e:DivisionByZeroError) {
    "Caught: "++e++"\n"
}

// Inheritance tests

try {
    raise NameError("")
} catch (e:Exception) {
    "Caught: "++e++"\n"
}

try {
    raise SyntaxError("")
} catch (e:ParserError) {
    "Caught: "++e++"\n"
}

try {
    raise IndexError("")
} catch (e:LookupError) {
    "Caught: "++e++"\n"
}

try {
    raise DivisionByZeroError("")
} catch (e:MathError) {
    "Caught: "++e++"\n"
}