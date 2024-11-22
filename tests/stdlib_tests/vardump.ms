a = 42
vardump(a)

// TODO: Add esc sequences
vardump("""
hello
\tthere""")
vardump("")

vardump(a > 4)

vardump(nil)

l = [1, "two", true, [8, 9]]
vardump(l)

vardump([])

enum MyEnum {
    A, B, C
}

vardump([[MyEnum]])

enum EmptyEnum {}

vardump(EmptyEnum)