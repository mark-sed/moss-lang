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

md"""
### Ternary if

Ternary if uses C-style syntax.
"""

condition ? "Yes\n" : "No\n"

md"""
## While and do-while loops

While and do-while, just like if, don't perform any implicit conversions to
bool.
"""

i = 0
while (i < 2) {
    "Ran while\n"
    i += 1
}

while (i == 0) "Will not run\n"

do {
    "Ran do while\n"
} while (i == -42)

md"""
## For loops

For works as a foreach and iterates over elements. But the C-style for loop
can be easily achieved using ranges.
"""

str = ""
for (i: 1,3..11) {
    str ++= i ++ " "
}
str++"\n"

sum = 0
for(s: [4,10,1,3]) {
    sum += s
}
sum++"\n"

md"""
## Switch statement

Switch does not fallthrough, but case can contain multiple values.
"""

fun test_switch(val) {
    x = 0
    switch(val) {
        case 1, 0, 4: return true
        case 8: return false
        case 10: { 
            x += 1
            return nil
        }
        case 11: return nil
        default: raise ValueError(val)
    }
}

test_switch(4)

md"""
## Imports

Import can appear pretty much anywhere and this import is valid within the
scope it appears in. The imported value can also be aliased using `as` keyword.
"""

import time
import time.localtime as lt, time.strftime as tsf
import sys as S

md"""
You can also import all of the symbols from the module and spill them into
the current scope.
"""

import sys.*

md"""
Once import is encountered this module is run.

You can also import/spill global spaces, but the space name (or full scope) has
to be prefixed with global scope specifier `::`, otherwise Moss would look for
a module with this name.
"""

space SomeSpace {
    fun foo() { "hi from foo\n"; }
    space Space2 {
        fun bar() { "hello from Space2 bar\n"; }
        fun baz() { "bye from Space2 baz\n"; }
    }
}

import ::SomeSpace.*
import ::SomeSpace.Space2.baz

~foo()
~baz()
~Space2.bar()

md"""
## Exception handling

Exception can be raised using `raise` keyword and the raised value can be any
object.

Although Moss will always raise only objects that extend class `Exception`.

Exceptions can be caught using `try` and `catch` block and can contain finally
block, which is always executed.
"""

fun some_fun(a) {
    raise a
}

try {
    some_fun(42)
} catch (e:NameError) {
    "Could not find function\n"
} catch (e) {
    e ++ " was raised\n"
} finally {
    "Finally is always done\n"
}

md"""
## Assert

Assertion takes a boolean value and if it is false, then AssertionError is
raised. Second argument can be string which will be part of the exception.
"""

x = 42
try
    assert(x < 0, "x should be negative")
catch (e:AssertionError)
    e

md"""
Assert is a keyword and cannot be overwritten.
"""
