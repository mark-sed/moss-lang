
# Syntax and constructs

Moss syntax aims to be simple and easy to read, but at the same time tries
to allow for quick writing of scripts and "one-liners".
Because of this, Moss uses `{}` (instead of indentations) and allows to end
an expression with either a new line (preferred for normal programs) or `;`
(needed for one-liners).
This means that new lines matter (and you can think of them as having
a `;` in their place).

## Variables

First assignment to a variable is treated as its declaration.
```moss
greet = "Hi\n"
```
```moss
greet // Will be outputted
```
_[Output]:_
```
Hi
```

Variable can also be assigned a value and even multiple at once:
```moss
a = b = c = 42
```

It is also possible to unpack values with assignment. Number of elements has to
match, or if one of the arguments contains `...` before it it will contain the
rest.
```moss
fun get_list() {
    return [1, 2, 3, 4, 5]
}
```
```moss
e, f, ...g = get_list()
```
```moss
e ++ ", " ++ f ++ ", " ++ g ++ "\n"
```
_[Output]:_
```
1, 2, \[3, 4, 5\]
```
```moss
h, ...i, j = get_list()
```
```moss
h ++ ", " ++ i ++ ", " ++ j ++ "\n"
```
_[Output]:_
```
1, \[2, 3, 4\], 5
```

If you want to access some global value that is overshadowed by a local
name, you can do that using the global scope - `::`.
```moss
num = 8
```
```moss
fun add_num(num) {
    return ::num + num
}
```
```moss
add_num(2)
```
_[Output]:_
```
10
```

If you want to access non-local variable overshadowed by a local one you
can use `$`.
```moss
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
```
```moss
non_local_x() // 5
```
_[Output]:_
```
5
```

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

## If statements

If has to be followed by a code block or an expression. The if condition has to
be a bool value and it will not implicitly convert other types to bool (to
avoid unwanted conversions and resulting bugs).
```moss
condition = 4 < 42
```
```moss
if (condition) {
    // Code
}

if (condition) "Smaller\n"
```
```moss
if (condition) "Smaller\n"
else "Bigger\n"

```
_[Output]:_
```
Smaller
```
```moss
if (num < 0) {
    // Code
} else if (num > 10) {
    // Code
} else {
    // Code
}
```

### Ternary if

Ternary if uses C-style syntax.
```moss
condition ? "Yes\n" : "No\n"
```
_[Output]:_
```
Yes
```

## While and do-while loops

While and do-while, just like if, don't perform any implicit conversions to
bool.
```moss
i = 0
```
```moss
while (i < 2) {
    "Ran while\n"
    i += 1
}
```
_[Output]:_
```
Ran while
```
_[Output]:_
```
Ran while
```
```moss
while (i == 0) "Will not run\n"

```
```moss
do {
    "Ran do while\n"
} while (i == -42)
```
_[Output]:_
```
Ran do while
```

## For loops

For works as a foreach and iterates over elements. But the C-style for loop
can be easily achieved using ranges.
```moss
str = ""
```
```moss
for (i: 1,3..11) {
    str ++= i ++ " "
}
```
```moss
str++"\n"
```
_[Output]:_
```
1 3 5 7 9 
```
```moss
sum = 0
```
```moss
for(s: [4,10,1,3]) {
    sum += s
}
```
```moss
sum++"\n"
```
_[Output]:_
```
18
```

## Switch statement

Switch does not fallthrough, but case can contain multiple values.
```moss
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
```
```moss
test_switch(4)
```
_[Output]:_
```
true
```

## Imports

Import can appear pretty much anywhere and this import is valid within the
scope it appears in. The imported value can also be aliased using `as` keyword.
```moss
import time
```
```moss
import time.localtime as lt, time.strftime as tsf
```
```moss
import sys as S
```

You can also import all of the symbols from the module and spill them into
the current scope.
```moss
import sys.*
```

Once import is encountered this module is run.

You can also import/spill global spaces, but the space name (or full scope) has
to be prefixed with global scope specifier `::`, otherwise Moss would look for
a module with this name.
```moss
space SomeSpace {
    fun foo() { "hi from foo\n"; }
    space Space2 {
        fun bar() { "hello from Space2 bar\n"; }
        fun baz() { "bye from Space2 baz\n"; }
    }
}
```
```moss
import ::SomeSpace.*
```
```moss
import ::SomeSpace.Space2.baz
```
```moss
~foo()
```
_[Output]:_
```
hi from foo
```
```moss
~baz()
```
_[Output]:_
```
bye from Space2 baz
```
```moss
~Space2.bar()
```
_[Output]:_
```
hello from Space2 bar
```

## Exception handling

Exception can be raised using `raise` keyword and the raised value can be any
object.

Although Moss will always raise only objects that extend class `Exception`.

Exceptions can be caught using `try` and `catch` block and can contain finally
block, which is always executed.
```moss
fun some_fun(a) {
    raise a
}
```
```moss
try {
    some_fun(42)
} catch (e:NameError) {
    "Could not find function\n"
} catch (e) {
    e ++ " was raised\n"
} finally {
    "Finally is always done\n"
}
```
_[Output]:_
```
42 was raised
```
_[Output]:_
```
Finally is always done
```

## Assert

Assertion takes a boolean value and if it is false, then AssertionError is
raised. Second argument can be string which will be part of the exception.
```moss
x = 42
```
```moss
try
    assert(x < 0, "x should be negative")
catch (e:AssertionError)
    e

md"""
```
_[Output]:_
```
AssertionError: x should be negative
```

Assert is a keyword and cannot be overwritten.
