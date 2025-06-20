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

```cpp
foo = "hi"
foo // Will be outputted
```

Variable can also be assigned a value and even multiple at once:
```cpp
a = b = c = 42
```

It is also possible to unpack values with assignment:
```cpp
fun foo() {
    return [1, 2, 3, 4, 5]
}

e, f[0].a, g = foo()

e // 1 
f[0].a // 2
g // [3, 4, 5]
```

If you want to access some global value that is overshadowed by a local
name, you can do that using the global scope - `::`.
```cpp
x = 8

fun foo(x) {
    return ::x + x
}
```

If you want to access non-local variable overshadowed by a local one you
can use `$`.

```cpp
x = 8
fun foo() {
    x = 4
    fun bar() {
        // Without $, this would create a new
        // variable x and x in foo would not
        // be overwritten
        $x = 5
    }
    bar()
    return x // 5
}
```

## Comments

Moss has line comments with Python/Bash syntax and block comments with C
syntax.

Line comments start from `//` until the end of the line (it has to be
a new line not `;`).

Block comments starts with `/*` and ends with first occurrence of `*/`.

```c
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
"Hello, World!"
```

## If statement

If has to be followed by a code block or an expression. The if condition has to
be a bool value and it will not implicitly convert other types to bool (to
avoid unwanted conversions and resulting bugs).

```py
if (condition) {
    // Code
}
```

```py
if (condition) expression
```

```py
if (condition) expression
else expression
```

```py
if (condition) {
    // Code
} else if (condition) {
    // Code
} else {
    // Code
}
```

### Ternary if

Ternary if uses C-style syntax.

```c
a = x ? 4 : 6
```

## While and do-while statement

While and do-while also don't perform any implicit conversions to bool.

```c
while (condition) {
    // Code
}
```

```c
while (condition) expr
```

```c
do {
    // Code
} while (condition)
```

```c
do expr; while (condition)
```

## For statement

For works as a foreach and iterates over elements. But the C-style for loop
can be easily achieved using ranges.

```py
for(i : 1,3..11) i // Prints:  13579
```

```py
for(s: "Hello there") {
    // Code
}
```

## Switch statement

Switch does not fallthrough, but case can contain multiple values.

```go
switch(val) {
    case 1, 0, 4: return true
    case 8: return false
    case 10: { 
        x += 1
        return nil
    }
    case 11: return nil
    default: error()
}
```

## Imports

Import can appear anywhere and is valid for the scope it appears in. Import can
have alias.

```py
import Math
import Statistics as st, Models as mod
import Math.Models as mods
```

Imported can be also all of the symbols
```py
import Rng.*
```

Once import is encountered this module is run, but not as a "main".

You can also import/spill global spaces, but the space name (or full scope) has
to be prefixed with global scope specifier `::`, otherwise Moss would look for
a module with this name.

```cpp

space SomeSpace {
    fun foo() { print("hi"); }
    space Space2 {
        fun bar() { print("ok"); }
        fun baz() { print("bye"); }
    }
}

import ::SomeSpace.*
import ::SomeSpace.Space2.baz

foo() // Foo is now accessible without scope
baz() // Also does not require scope
Space2.bar() // Requires scope for space2, since only baz was imported
```


## Exception handling

Exception can be raised using `raise` keyword and it can be any object.

```py
fun bar() {
    raise 42
}
```

```py
fun baz() {
    raise KnownException("Oops")
}
```

It can be caught using `try` and `catch` block and can be ended with finally
block, which is always executed.

```java
try {
    someCall()
} catch (e:KnownException) {
    e.nicemsg()
} catch (e) {
    e
} finally {
    "This is always done"
}

```

## Assert

Assertion takes in a bool and if this bool is false it raises an assertion
exception. Second argument can be string which will be part of the exception.

Assertions are ignored when debug is turned off.

```py
assert(x > 0)
assert(y < 0, "Input value should be negative.")
```

Assert does have a function syntax, but is in fact a keyword and cannot be
overwritten.

## Short-circuit evaluation

`||` and `&&` are logical or and end respectivelly, but don't necessairly
evaluate their second argument.

These operators can be used to create an evaluation chain.

```cpp
fun foo() { return false; }

foo() || exit(1) && print("success")
```

This will exit the program with return code 1.

`||` can be thought of as the next evaluation after a false value and
`&&` as the next one after a true value.
