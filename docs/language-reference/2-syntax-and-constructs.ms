@!enable_code_output

md"""
# Syntax and constructs

Moss syntax aims to be simple and easy to read, but at the same time tries
to allow for quick writing of scripts and "one-liners".
Because of this, Moss uses `{}` (instead of indentations) and allows to end
an expression with either a new line (preferred for normal programs) or `;`
(needed for one-liners).
This means that new lines matter (and you can think of them as having
a `;` in their place).
"""

md"""
## Variables

First assignment to a variable is treated as its declaration.
"""

greet = "Hi\n"
greet // Will be outputted

md"""
Variable can also be assigned a value and even multiple at once:
"""

a = b = c = 42

md"""
It is also possible to unpack values with assignment. Number of elements has to
match, or if one of the arguments contains `...` before it it will contain the
rest.
"""

fun get_list() {
    return [1, 2, 3, 4, 5]
}

e, f, ...g = get_list()
e ++ ", " ++ f ++ ", " ++ g ++ "\n"

h, ...i, j = get_list()
h ++ ", " ++ i ++ ", " ++ j ++ "\n"

md"""
If you want to access some global value that is overshadowed by a local
name, you can do that using the global scope - `::`.
"""

num = 8
fun add_num(num) {
    return ::num + num
}
add_num(2)

md"""
If you want to access non-local variable overshadowed by a local one you
can use `$`.
"""

fun non_local_x() {
    x = 4
    fun bar() {
        // Without $, this would create a new
        // variable x and x in foo would not
        // be overwritten
        $x = 5
    }
    ~bar()
    return x
}
non_local_x() // 5

md"""
## Comments

Moss uses C style comments. Line comments start from `//` until the end of the
line (it has to be a new line not `;`).

Block comments starts with `/*` and ends with first occurrence of `*/`.
```cpp
// Line comment; Ends with a new line

/*
Block comment
*/
```

### Shebang

Moss recognizes shebang to allow for direct execution of scripts. The shebang
can appear only on the first line and moss treats it like a one line comment.

```py
#!/usr/bin/moss
```
"""

md"""
## If statements

If has to be followed by a code block or an expression. The if condition has to
be a bool value and it will not implicitly convert other types to bool (to
avoid unwanted conversions and resulting bugs).
"""

condition = 4 < 42

if (condition) {
    // Code
}

if (condition) "Smaller\n"
else "Bigger\n"

if (num < 0) {
    // Code
} else if (num > 10) {
    // Code
} else {
    // Code
}