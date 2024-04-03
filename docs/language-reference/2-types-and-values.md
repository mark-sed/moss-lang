# Types and values

Moss offers the basic built-in types and then a way to define custom ones.
The main approach is using object oriented programming.

Every value can be worked with as an object and therefore call its methods.
E.g.:

```py
"moss".upper() # "MOSS"
[1,2,3].len()  # 3
1.(+)(2)       # "3"
```

## Int

Signed integer value.

Integer constant can be written in decimal, hexadecimal, octal or binary value:

```py
dec_i = 42       # Decimal
hex_i = 0x2a     # Hexadecimal
oct_i = 0q52     # Octal
bin_i = 0b101010 # Binary
```

## Float

Floating point value.

Float constant can be written in a standard form with `.`, but has to start
with a number and has to be followed by a number (to avoid issues with
operators). Floats can also be written in a scientific format.

```py
x = -3.1415925
y = 0.5e-2      # 0.005
z = 5e3         # 500.0
```

## Bool

Boolean value.

Bool can be either `true` or `false`.

```py
b1 = true
b2 = false

b3 = b1 or b2
b4 = b1 == true
```

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

```py
name = "Jon"
lang = "Moss"
msg = f"{name} loves coding in {lang}"
msg # "Jon loves coding in Moss"
```

Custom prefixes can be used and this can be very useful mainly for output file
generation. This approach will be discussed later on.

String can also be written on multiple lines using `"""`, this is the same
as when using `"`, but there can be new lines, which will be then present
in the string itself.

```py
help = """Program usage:
\tmoss [interpret options] <input file> [program args]
"""
help # "Program usage:\n\tmoss [interpret options] <input file> [program args]\n"
```

## Nil and NilType

A special value is the `nil` value, which has type `NilType` and it's the only
value that this type can have. Any variable that has not been assigned a
value will be of this type, bit it can also be assigned to it.

```py
a       # nil
b = nil # nil
a == b  # true
```

## Enum

Enumeration type.

```cpp
enum COLORS {
    PURPLE,
    YELLOW,
    GREEN
}

flower = COLORS::PURPLE
moss = COLORS::GREEN
```

## List

A dynamic size, heterogenous, ordered, sequence of values.

```py
names = ["luke", "leia", "han"]
mix = ["pirate", 42, true, nil, nil, "ok"]
```

## Dict

Dictionary - an associative array - collection of keys and values.

```py
empty = {,}
mappings = {"name": "Marek", "id": 42}
```
