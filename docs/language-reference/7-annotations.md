# Annotations

Annotations serve mostly for interaction with the interpreter.

Annotations start with `@` (or `@!` for inner annotations) and are followed by
the annotation name and any possible arguments.

There are 2 types of annotations - inner and outer.
Inner annotations (`@!`) are tied to the what ever they are declared inside of
and are mostly important for declaring module attributes. The outer annotation
is tied to what ever follows it.

```cpp
@!min_version("1.1") // Module annotation

@equation // Tied to `f`
@hidden   // Tied fo `f`
fun f(x) {
    return x ^ 2 * x
}

fun g(x) {
    @!equation // Tied to `g`
    @!hidden   // Tied fo `g`
    return x * x
}
```

## Internal representation

Annotation has a name and a value. Name is strictly an identificator (string)
and value is a valid moss value or comma separated list of values, which
will be converted into a list.
Explicit value (for annotation without parenthesis) is `nil`.