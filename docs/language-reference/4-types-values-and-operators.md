# Types, values and operators

Moss offers the basic built-in types and then a way to define custom ones.
The main approach is using object oriented programming.

Every value can be worked with as an object and therefore call its methods.
E.g.:

```cpp
"moss".upper() // "MOSS"
[1,2,3].len()  // 3
1.(+)(2)       // "3"
```

> __Note__: Every class and type by default overrides these methods:
> `fun id()`, conversion to string, `fun (++)(x)`, `fun (==)(x)` and
> `fun (!=)(x)`.

## Int

Signed integer value.

Integer constant can be written in decimal, hexadecimal, octal or binary value:

```cpp
dec_i = 42       // Decimal
hex_i = 0x2a     // Hexadecimal
oct_i = 0q52     // Octal
bin_i = 0b101010 // Binary

// x q or b can be also capitalized
bin_j = 0B0101
```

__Class constructors__:
* `fun Int(v:[Int, Float, Bool, String])`
* `fun Int(v=0)`

__Class operators__:
* `fun (+)(x:[Int, Float, Complex])`
* `fun (-)(x:[Int, Float, Complex])`
* `fun (*)(x:[Int, Float, Complex])`
* `fun (/)(x:[Int, Float, Complex])`
* `fun (%)(x:[Int, Float, Complex])`
* `fun (^)(x:[Int, Float, Complex])`
* `fun (==)(x:[Int, Float, Complex])`
* `fun (!=)(x:[Int, Float, Complex])`
* `fun (<=)(x:[Int, Float, Complex])`
* `fun (>=)(x:[Int, Float, Complex])`
* `fun (<)(x:[Int, Float, Complex])`
* `fun (>)(x:[Int, Float, Complex])`
* `fun (and)(x:Int)`
* `fun (or)(x:Int)`
* `fun (xor)(x:Int)`
* `fun (not)()`

__Class methods__:


## Float

Floating point value.

Float constant can be written in a standard form with `.`, but has to start
with a number and has to be followed by a number (to avoid issues with
operators). Floats can also be written in a scientific format.

```cpp
x = -3.1415925
y = 0.5e-2      // 0.005
z = 5e3         // 500.0
j = 0.e+3       // 0.0

not_a_number = Float("nan")
infinity_value = Float("inf")
```

__Class constructors__:
* `fun Float(v:[Int, Float, Bool, String])`
* `fun Float(v=0.0)`

__Class operators__:
* `fun (+)(x:[Int, Float, Complex])`
* `fun (-)(x:[Int, Float, Complex])`
* `fun (*)(x:[Int, Float, Complex])`
* `fun (/)(x:[Int, Float, Complex])`
* `fun (%)(x:[Int, Float, Complex])`
* `fun (^)(x:[Int, Float, Complex])`
* `fun (==)(x:[Int, Float, Complex])`
* `fun (!=)(x:[Int, Float, Complex])`
* `fun (<=)(x:[Int, Float, Complex])`
* `fun (>=)(x:[Int, Float, Complex])`
* `fun (<)(x:[Int, Float, Complex])`
* `fun (>)(x:[Int, Float, Complex])`

__Class methods__:
* `fun is_nan()`
* `fun is_inf()`

## Complex

Complex number value.

When converting from String the value cannot have whitespace characters and must
use `i` for the imaginary part.

```cpp
a = Complex(1.2, 3) // 1.2+3i
b = Complex(1)      // 1+0i
c = Complex("2-1i") // 2-1i

// d = Complex("3 + 2i") is INCORRECT format
```

__Class constructors__:
* `fun Complex(r:[Int, Float]=0, i:[Int, Float]=0)`
* `fun Complex(c:String)`
* `fun Complex(c:Complex)`

__Class operators__:
* `fun (+)(x:[Int, Float, Complex])`
* `fun (-)(x:[Int, Float, Complex])`
* `fun (*)(x:[Int, Float, Complex])`
* `fun (/)(x:[Int, Float, Complex])`
* `fun (%)(x:[Int, Float, Complex])`
* `fun (^)(x:[Int, Float, Complex])`
* `fun (==)(x:[Int, Float, Complex])`
* `fun (!=)(x:[Int, Float, Complex])`
* `fun (<=)(x:[Int, Float, Complex])`
* `fun (>=)(x:[Int, Float, Complex])`
* `fun (<)(x:[Int, Float, Complex])`
* `fun (>)(x:[Int, Float, Complex])`

__Class methods__:

## Bool

Boolean value.

Bool can be either `true` or `false`.

```cpp
b1 = true
b2 = false

b3 = b1 or b2
b4 = b1 == true
```

__Class constructors__:
* `fun Bool(v:[Int, Float, Bool, NilType])`
* `fun Bool(v=false)`

__Class operators__:
* `fun (and)(x:Bool)`
* `fun (or)(x:Bool)`
* `fun (xor)(x:Bool)`
* `fun (not)()`
* `fun (&&)(x)`
* `fun (||)(x)`
* `fun (==)(x:bool)`
* `fun (!=)(x:bool)`

__Class methods__:

## String and xString

String of UNICODE symbols.

xString is a name for string constant with a prefix, this prefix serves to
determine how to work and modify this string. Simplest example of an xString
is `rString` (Raw String):

```py
nl = r"New line is written as \n"
```

In the example above the `\n` will be in the string as is not parsed as an
escape sequence. Another example is the `fString` (Formatted String), which
uses string interpolation for the construction of the string:

```cpp
name = "Jon"
lang = "Moss"
msg = f"{name} loves coding in {lang}"
msg // "Jon loves coding in Moss"
```

Custom prefixes can be used and this can be very useful mainly for output file
generation. This approach will be discussed later on.

String can also be written on multiple lines using `"""`, this is the same
as when using `"`, but there can be new lines, which will be then present
in the string itself.

```cpp
help = """Program usage:
\tmoss [interpret options] <input file> [program args]
"""
help // "Program usage:\n\tmoss [interpret options] <input file> [program args]\n"
```

__Class constructors__:
* `fun String(v)`
* `fun String(v="")`

__Class operators__:
* `fun (+)(x:String)`
* `fun (in)(x:String)`
* `fun ([])(x:[Int, Range])`
* `fun (==)(x:String)`
* `fun (!=)(x:String)`
* `fun (<=)(x:String)`
* `fun (>=)(x:String)`
* `fun (<)(x:String)`
* `fun (>)(x:String)`

__Class methods__:
* `fun len()`
* `fun repeat(x:Int)`
* `fun upper()`
* `fun lower()`
* `fun capitalize()`
* `fun reverse()`
* `fun find(substr:String)`
* `fun empty()`
* `fun escape()`
* `fun unescape()`

## Nil and NilType

A special value is the `nil` value, which has type `NilType` and it's the only
value that this type can have. Any variable that has not been assigned a
value will be of this type, bit it can also be assigned to it.

```cpp
a       // nil
b = nil // nil
a == b  // true
```

__Class constructors__:
* `fun NilType()`

## Enum

Enumeration type.

```cpp
enum COLORS {
    PURPLE,
    YELLOW,
    GREEN
}

flower = COLORS.PURPLE
moss = COLORS.GREEN
```

## List

A dynamic size, heterogenous, ordered, sequence of values.

```py
names = ["luke", "leia", "han"]
mix = ["pirate", 42, true, nil, nil, "ok"]
```

__Class constructors__:
* `fun List(.. v)`

__Class operators__:
* `fun (+)(x:List)`
* `fun (in)(x)`
* `fun ([])(x:[Int, Range])`
* `fun (==)(x)`
* `fun (!=)(x)`

__Class methods__:
* `fun append(x)`
* `fun len()`
* `fun reverse()`
* `fun find(x)`
* `fun sort(bool ascend=true)`
* `fun remove(index:Int)`
* `fun remove_all(x)`
* `fun remove_first(x)`
* `fun remove_last(x)`
* `fun remove_if(f)`
* `fun push(x)`
* `fun pop()`
* `fun filter(f)`
* `fun empty()`
* `fun insert(index:Int, x)`
* `fun count(x)`

### List comprehension

List comprehension can be used to create custom lists using something that
resembles mathematical notation and would otherwise require a loop and
appending to a list.

The syntax for unconditioned is as follows:

_[expression : variable = expression < comma separated more assignments >]_

The conditioned comprehension is as follows:

_[expression if(condition) < else expression > : variable = expression < comma separated more assignments >]_

```cpp
l1 = [1, 2, 3]
l2 = [4, 5, 6]
ziped = [[x, y] : x = l1, y = l2] 
// [[1, 4],[2, 5],[3, 6]]

[a : a = (1,3..100)] // odd numbers from 1 to 99

[p if(all([p % x != 0 : x = (2..p/2)])) : p = (2..1000)] // primes

greet = "Hello there programmer!"
[c if(c != " ") else "_" : c = greet].join() // "Hello_there_programmer!"
// Alternatively one can use ternary operator for this case as well
[(c != " ") ? c : "_" : c = greet].join() // "Hello_there_programmer!"
```

## Dict

Dictionary - an associative array - collection of keys and values.

```py
empty = {:}
mappings = {"name": "Marek", "id": 42}
```

__Class constructors__:
* `fun Dict()`
* `fun Dict(keys:List, values:List)`

__Class operators__:
* `fun (in)(x)`
* `fun ([])(x)`

__Class methods__:
* `fun keys()`
* `fun values()`
* `fun find(key)`
* `fun remove(key)`
* `fun remove_if(f)`
* `fun filter(f)`
* `fun empty()`
* `fun len()`
* `fun insert(key, value)`
* `fun emplace(key, value)`

## Range

Range is the result for range construct (`1,3..10`), which can be used
for easy iterator or sequence creation.

Beware that in calls and subscripts it is highly recommended to surround ranges
in parenthesis as arguments have precedence over ranges, meaning that the
first comma after a value in a function call will be taken as an argument
separator and not a second value in a range:

```c
foo(1,2..5) // this is equivalent to foo(1, (2..5))
```

__Class constructors__:
* `fun Range(start, end, step=1)`

__Class operators__:
* `fun (in)(x)`
* `fun ([])(x)`

__Class methods__:
* `fun __next()`

# Operators

| **Operator**                     | **Description**                                  | **Associativity** |
|----------------------------------|--------------------------------------------------|---------------|
| `::`                             | Global scope resolution                          | Left       |
| `()`                             | Function call                                    | Left          |
| `[]`                             | List Selection                                   | Left          |
| `.`                              | Object element selection                         | Left          |
| `+`, `-`                         | Unary + and -                                    | Right         |
| `^`                              | Exponentiation                                   | Right         |
| `*`, `/`, `%`                    | Multiplication, Division, Modulo                 | Left          |
| `+`, `-`                         | Addition, Subtraction                            | Left          |
| `++`                             | Concatenation                                    | Left          |
| `..`                             | Range                                            | Left          |
| `in`                             | Membership                                       | Left          |
| `<=`, `>=`, `>`, `<`             | Comparisons                                      | Left          |
| `==`, `!=`                       | Equals, Not equals                               | Left          |
| `not`                            | Logical not                                      | Right         |
| `&&`, `and`                      | Short-circuit and logical/bitwise and            | Left          |
| `\|\|`, `or`                     | Short-circuit and logical/bitwise or             | Left          |
| `xor`                            | Logical/bitwise xor                              | Left          |
| `?:`                             | Ternary if                                       | Right         |
| `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `^=`, `++=` | Assignment, Operation and assignment | Right       |
| `~`                              | Silence operator                                 | Right         |

> __Note:__ Unpack "operator" (`<<`) is not an actual operator, it is a special
> syntax for function calls and cannot appear in expressions nor can be
> overriden for a class.