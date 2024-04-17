# Functions

Functions allow for overloading by specifying parameter type. Function always
returns a value, if not specified then it returns a `nil`.

* Functions can be nested.
* Functions can contain spaces.
* Functions can contain classes.
* Functions can contain imports.

```cpp
fun foo(arg1, arg2) {
    return arg1 * arg2
}
```

Moss has also __lambdas__, but you should not think of them as something other
than "normal" functions, instead of having a block of code, they have an `=`
symbol followed by a value/expression that will be returned.

There can be also anonymous functions (lambdas).

```
fun foo(arg1) = arg1 * 2

fun(arg1) = arg1 * 2
```

Arguments can have their type specified. The type specification can be also
a list of possible types.

```cpp
fun getID(x:MyClass) {
    return -1
}

fun getID(x:[Int, Bool]) {
    return 0
}

fun getID(x) {
    return 42
}
```

Arguments can also have default value. The default value does not set the
argument type, that has to be set explicitly.

```cpp
fun bar(a:Int=4) {
    // Code
}
```

Variable amount of arguments can be also specified and can be followed by named
arguments, that have to have default value. Variable arguments will then be
stored in a list. The type cannot be condition for variable arguments as it is
always converted to a list:

```cpp
fun baz(a, b:Int, ... other, name="") {
    // Code
}
```

## Operator functions

Classes can override operators by overriding operator functions.

These functions are called by the operator symbol enclosed in parentheses.

The arguments can be specialized.

```cpp
class MyClass {

    fun (+)(x:[Int,Float]) = this.x + x
    fun (+)(x) {
        error("Cannot add to MyClass type " ++ type(x))
    }

    fun (and)(x) = this.x > 0

}
```