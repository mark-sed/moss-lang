# Classes and spaces

Classes and spaces are 2 constructs for setting the scope of resources.

## Spaces

Space is just a scope and accessibility construct. They encapsulate symbols 
within it into its own scope. It can be named or anonymous.

```cpp
space Math {
    fun abs(x:Int) {
        return x 
    }
}

space {
    a = someCall()
    a // A is printed and unaccessible
}
```

## Classes 

Classes allow for multiple inheritance and all function in their body have
hidden `this` variable, which accesses the object it belongs to.

Class can contain multiple constructors (functions that have the same name as
the class)

```cpp
class Range : Iterable, BaseClass {
    fun Range(start, end, step=1) {
        this.start = start
        this.end = end
        this.step = step
        this.i = start
    }

    fun __next() {
        if(this.step >= 0 and this.i >= this.end) return StopIteration
        if(this.step < 0 and this.i <= this.end) return StopIteration
        r = this.i
        this.i += this.step
        return r
    }
}
```

When calling methods on static classes, not objects, `this` argument has to be
passed in as the first one:

```cpp
Range::__next(range)
```

### Class default methods

These methods are internally used for conversion or control flow, such as
`__next` for iterations.

All these methods begin with `__`.

List of default class methods excluding operators, which are mentioned in
other document section:

* `fun __iterator()` - Used to get the object that will be `__next` called on.
It is called at the beginning of an iteration.
* `fun __next()` - Called to get the next value in an iteration. Once the end
is reached, an exception is raised.

### Class constructor

Constructor has to have the same name as the class:

```cpp
class Foo {
    fun Foo(id:Int) {
        this.id = id
    }

    fun Foo(id) {
        this.id = hash(id)
    }
}
```

If there is no constructor then one with no arguments is created implicitly (but not once one is created).

### Calling class methods

When calling a method on class not an object one can pass the object as the last argument:

```cpp
class Foo {
    fun doit(num) {
        this.num = num
    }
}

foo = Foo()
Foo.doit(67, foo) // This is the same as `foo.doit(67)`
```