# Notes

"Note" is a term that refers to any expression that will be outputted
to the output file ("document"). The document and its format is chosen
by the user then running the interpreter and therefore it might not be
even created or it can even be the standard output.

Any right hand side expression is considered a note. The most common and simple
one is just a string literal.

```py
"This will be in document"

"""
This multiline
string as well.
"""
```

But it can be also things like function calls. In Moss any value has a to string
converting function, and that will be always called on a non-string note value.

```py
fun foo() {
    return 42
}

foo() # 42 will be in the output file
```

```py
class MyClass {
    # Code
}

MyClass() # Will be converted to string and outputted
```

If one wants to call a function and disregard the return value, the
"silent" operator `~` can be used.

```py
~foo() # 42 will not be in the output file
```

> Note: For one-liners, using notes as prints is quite suitable and saves
> calls to `print`.

