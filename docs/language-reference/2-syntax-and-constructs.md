# Syntax and constructs

Moss syntax aims to be simple and easy to read, but at the same time tries
to allow for quick writing of scripts and "one-liners".
Because of this, Moss uses `{}` (instead of indentations) and allows to end
an expression with either a new line (preferred for normal programs) or `;`
(needed for one-liners).
This means that new lines matter (and you can think of them as having
a `;` in their place).

## Variables

Any symbol name is a variable declaration, if not yet declared, then it
is a note.

```py
foo # foo = nil
foo # Will be outputted as "nil"
```

Variable can of course be also assigned a value and even multiple at once:
```py
a = b = c = 42
```

It is also possible to unpack values with assignment:
```py
fun foo() {
    return [1, 2, 3, 4, 5]
}

e, f, g = foo()

e # 1 
f # 2
g # [3, 4, 5]
```

If you want to access some global value that is overshadowed by a local
name, you can do that using the global scope - `::`.
```py
x = 8

fun foo(x) {
    return ::x + x
}
```

If you want to access non-local variable overshadowed by a local one you
can use `$`.

```py
x = 8
fun foo() {
    x = 4
    fun bar() {
        # Without $, this would create a new
        # variable x and x in foo would not
        # be overwritten
        $x = 5
    }
    bar()
    return x # 5
}
```

## Comments

Moss has line comments with Python/Bash syntax and block comments with C
syntax.

Line comments start from `#` until the end of the line (it has to be
a new line not `;`).

Block comments starts with `/*` and ends with first occurrence of `*/`.

```c
# Line comment; Ends with a new line

/*
Block comment
*/
```

## If statement

If has to be followed by a code block or an expression. The if condition has to
be a bool value and it will not implicitly convert other types to bool (to
avoid unwanted conversions and resulting bugs).

```py
if (condition) {
    # Code
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
    # Code
} else if (condition) {
    # Code
} else {
    # Code
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
    # Code
}
```

```c
while (condition) expr
```

```c
do {
    # Code
} while (condition)
```

```c
do expr while (condition)
```

## For statement

For works as a foreach and iterates over elements. But the C-style for loop
can be easily achieved using ranges.

```py
for(i : 1,3..11) i # Prints:  13579
```

```py
for(s: "Hello there") {
    # Code
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

```
import Math
import Statistics as st, Models as mod
import Math::Rnd as rng
```

Imported can be also all of the symbols
```
import Rng::*
```

Once import is encountered this module is run, but not as a "main".

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