# Bytecode and program sharing

Bytecode can be used as mean for program sharing without sharing actual moss
source code. This is similar to Java's approach and sharing a .jar file.

Constants and values are stored in memory pool and the bytecode (unlike Java
or CPython) is register based.

Bytecode is stored in little endian byte order (with byte being 8 bits).

## Memory pools

There are multiple memory pools

### Constant memory pool

This pool is populated during parsing or loading of bytecode. This pool
variables (registers) are prefixed by `#`.

```
STORE_CONST #0, "Hello, World!"
OUTPUT      #0
```

Commonly used constants are pushed into this pool by default:
- 0 - nil
- 1 - 99 - int values 
- 100 - float 0.0
- 101 - float 1.0
- 102 - float +inf
- 103 - float -inf
- 104 - float nan
- 106 - true
- 107 - false

### Register and local variables memory pool

This pool holds temporary values and local variables these values can be
accessed with `%` prefix.

```
ADD     %3, 42, %0
SUB     %2, %3, %1
```

## Bytecode memory model

__Sizes__:
* Opcode - 1 B
* Register - 4 B 
* Int value - 8 B
* Float value - 8 B
* Address - 4 B
* Strings - 4 B for its byte length (UNICODE, so not character length) followed by that many bytes

## Bytecode header

## Opcodes

Strings are stored with their size followed by their value.

```
00h - END - End of code

01h - LOAD          %dst, "name"
02h - LOAD_ATTR     %dst, %src, "name"
03h - LOAD_GLOBAL   %dst, "name"
xxh - LOAD_NONLOC   %dst, "name"

xxh - STORE             %dst, %src
xxh - STORE_NAME        %dst, "name"
xxh - STORE_CONST       %dst, #val
xxh - STORE_ATTR        %src, %obj, "name"
xxh - STORE_CONST_ATTR  #val, %obj, "name"
xxh - STORE_GLOBAL      %src, "name"
xxh - STORE_NONLOC      %src, "name"
xxh - STORE_SUBSC       %src, %obj, %key
xxh - STORE_CONST_SUBSC #val, %obj, %key
xxh - STORE_SUBSC_CONST %src, %obj, #ckey
xxh - STORE_C_SUBSC_C   #val, %obj, #ckey

xxh - STORE_INT_CONST   #dst, int
xxh - STORE_FLOAT_CONST #dst, float
xxh - STORE_BOOL_CONST  #dst, bool
xxh - STORE_STR_CONST   #dst, "string"

xxh - JMP               addr
xxh - JMP_IF_TRUE       %src, addr
xxh - JMP_IF_FALSE      %src, addr
xxh - CALL              %dst, %src
xxh - PUSH_FRAME
xxh - POP_FRAME
xxh - PUSH_CALL_FRAME
xxh - POP_CALL_FRAME
xxh - RETURN            %val
xxh - RETURN_CONST      #val
xxh - PUSH_ARG          %val
xxh - PUSH_CONST_ARG    #val
xxh - PUSH_NAMED_ARG    %val, "name"
xxh - PUSH_UNPACKED     %val
xxh - CREATE_FUN        %fun, "name" "arg csv" // eg: "foo" "a,b,c,d"  
xxh - FUN_BEGIN         %fun
xxh - SET_DEFAULT       %fun, int, %src
xxh - SET_DEFAULT_CONST %fun, int, #src
xxh - SET_TYPE          %fun, int, %type
xxh - SET_VARARG        %fun, int

xxh - IMPORT        %dst, "name"
xxh - IMPORT_ALL    %src

xxh - PUSH_PARENT   %class
xxh - BUILD_CLASS   %dst, "name"

xxh - ANNOTATE      %dst, "name", %val
xxh - DOCUMENT      %dst, "txt"

xxh - OUTPUT    %src

xxh - CONCAT    %dst, %src1, %src2
xxh - EXP       %dst, %src1, %src2
xxh - ADD       %dst, %src1, %src2
xxh - SUB       %dst, %src1, %src2
xxh - DIV       %dst, %src1, %src2
xxh - MUL       %dst, %src1, %src2
xxh - MOD       %dst, %src1, %src2
xxh - EQ        %dst, %src1, %src2
xxh - NEQ       %dst, %src1, %src2
xxh - BT        %dst, %src1, %src2
xxh - LT        %dst, %src1, %src2
xxh - BEQ       %dst, %src1, %src2
xxh - LEQ       %dst, %src1, %src2
xxh - IN        %dst, %src1, %src2
xxh - AND       %dst, %src1, %src2
xxh - OR        %dst, %src1, %src2
xxh - NOT       %dst, %src1
xxh - NEG       %dst, %src1
xxh - XOR       %dst, %src1, %src2
xxh - SUBSC     %dst, %src, %index

xxh - CONCAT2   %dst, #val, %src2
xxh - EXP2      %dst, #val, %src2
xxh - ADD2      %dst, #val, %src2
xxh - SUB2      %dst, #val, %src2
xxh - DIV2      %dst, #val, %src2
xxh - MUL2      %dst, #val, %src2
xxh - MOD2      %dst, #val, %src2
xxh - EQ2       %dst, #val, %src2
xxh - NEQ2      %dst, #val, %src2
xxh - BT2       %dst, #val, %src2
xxh - LT2       %dst, #val, %src2
xxh - BEQ2      %dst, #val, %src2
xxh - LEQ2      %dst, #val, %src2
xxh - IN2       %dst, #val, %src2
xxh - AND2      %dst, #val, %src2
xxh - OR2       %dst, #val, %src2
xxh - XOR2      %dst, #val, %src2
xxh - SUBSC2    %dst, #src, %index

xxh - CONCAT3   %dst, %src1, #val
xxh - EXP3      %dst, %src1, #val
xxh - ADD3      %dst, %src1, #val
xxh - SUB3      %dst, %src1, #val
xxh - DIV3      %dst, %src1, #val
xxh - MUL3      %dst, %src1, #val
xxh - MOD3      %dst, %src1, #val
xxh - EQ3       %dst, %src1, #val
xxh - NEQ3      %dst, %src1, #val
xxh - BT3       %dst, %src1, #val
xxh - LT3       %dst, %src1, #val
xxh - BEQ3      %dst, %src1, #val
xxh - LEQ3      %dst, %src1, #val
xxh - IN3       %dst, %src1, #val
xxh - AND3      %dst, %src1, #val
xxh - OR3       %dst, %src1, #val
xxh - XOR3      %dst, %src1, #val
xxh - SUBSC3    %dst, %src, #index

xxh - ASSERT    %src, %msg

xxh - RAISE         %val
xxh - CATCH         "exc_name", addr
xxh - CATCH_TYPED   "exc_name", "type", addr
xxh - POP_CATCH

xxh - LIST_PUSH         %dst, %val
xxh - LIST_PUSH_CONST   %dst, #val
xxh - BUILD_LIST        %dst

xxh - BUILD_DICT        %dst, %keys, %vals
xxh - BUILD_ENUM        %dst, %vals, "name"
xxh - BUILD_SPACE       %dst, "name"

xxh - CREATE_RANGE      %dst, %start, %step, %end
xxh - CREATE_RANGE2     %dst, #start, %step, %end
xxh - CREATE_RANGE3     %dst, %start, #step, %end
xxh - CREATE_RANGE4     %dst, %start, %step, #end
xxh - CREATE_RANGE5     %dst, #start, #step, %end
xxh - CREATE_RANGE6     %dst, #start, %step, #end
xxh - CREATE_RANGE7     %dst, %start, #step, #end
xxh - CREATE_RANGE8     %dst, #start, #step, #end

xxh - SWITCH        %listvals, %listaddr, addr_def
xxh - FOR           %i, %iterator, addr
xxh - ITER          %iterator, %collection
```

## Examples

```py
import systemx as sx

a = sx.a + 430
b = 8
```

```
0   STORE_INT_CONST #200, 430
1   IMPORT      %0, "systemx"
    STORE_NAME  %0, "sx"

3   LOAD_ATTR   %3, %0(sx), "a"
3   ADD         %1(a), %3, #200

4   STORE_CONST %2(b), #8
```

When interpreted, STORE_CONST will be skipped as the vm will load
these values during parsing.

### Function

```cpp
fun foo(a, b:Int=3, ...c, d=4) {
    d"Does foo"
}

fun foo(a) {

}

foo(2, 4)
foo(true, 2, 1, 2, 3, d=8)
foo(false)
```

```
STORE_INT_CONST #0, 3
STORE_CONST     %0, #0
STORE_INT_CONST #1, 4
STORE_CONST     %1, #1

JMP  <after fun foo>
; This will create FunValue %2 will store "foo"
; all the other stuff will be parsed into this value as well
CREATE_FUN      %2, "foo" "a,b,c,d"  
SET_DEFAULT     %2, 1, %0
SET_TYPE        %2, 1, "Int"
SET_VARARG      %2, 2
SET_DEFAULT     %2, 3, %1
FUN_BEGIN       %2
; Body
POP_CALL_FRAME  %2
STORE_CONST_NIL #2
RETURN_CONST    #2

JMP <after this fun foo>
CREATE_FUN      %3, "foo(a)"
FUN_BEGIN       %3
POP_CALL_FRAME  %3

...
LOAD             %4, "foo" 
PUSH_CONST_ARG   #200 ;2
; Type will be checked on pointer/value bases
PUSH_CONST_ARG   #1   ;4
CALL             %5, %4
OUTPUT           %5

LOAD             %6, "foo"
PUSH_CONST_ARG   #201; true
PUSH_CONST_ARG   #202 
PUSH_CONST_ARG   #203 
PUSH_CONST_ARG   #204
PUSH_CONST_ARG   #205
STORE_CONST      %7, #206
PUSH_NAMED_ARG   %7, "d"
CALL             %6
```

### Classes

```cpp
class MyClass : XClass {

    NAME = "My Class"

    fun MyClass(a) {
        this.a = a
    }

    fun get_a() = this.a
}

myc = MyClass(42)
print(myc.get_a())
```

```
x   LOAD        %1, "XClass"
x   PUSH_PARENT %1
x   BUILD_CLASS %0, "MyClass"

x   STORE_CONST_ATTR  #200, %0, "NAME"

0   ;Constructor with no args
0   STORE_ATTR_ADDR <next bc>, %0, "MyClass()"
0   PUSH_FRAME
0   CREATE_OBJ  %0, %0
0   RETURN %0

x   STORE_ATTR_ADDR  <next bc>, %0, "MyClass(_)"
x   PUSH_FRAME
x   LOAD_GLOBAL %1, "XClass"
x   CALL        %2, %1
x   LOAD_GLOBAL %3, "MyClass"
x   STORE_ATTR  %0, %2(this), "a"
x   RETURN      %2

x   STORE_ATTR_ADDR  <next bc>, %0, "get_a(MyClass)"
x   PUSH_FRAME
x   LOAD_ATTR   %1, %0, "a"
x   RETURN      %1

x   LOAD_GLOBAL %2, "MyClass"
x   LOAD_ATTR   %3, "MyClass"
x   PUSH_ARG    #42
x   CALL        %4, %3
x   LOAD_ATTR   %5, %4, "get_a"
x   PUSH_ARG    %4
x   CALL        %6, %5
x   LOAD_GLOBAL %7, "print"
x   PUSH_ARG    %6
x   CALL        %8, %7
```

### Notes, docs and annotations

```py
md"""
# Project Moss
Very cool stuff!
"""

@requires(1)
fun foo() {
    d"""
    Does nothing
    """
}
```

```
x   STORE_STR_CONST #200, "# Project Moss\nVery cool stuff!\n"
x   STORE_STR_CONST #201, "\nDoes nothing\n"

x   LOAD_GLOBAL     %2, "md"
x   PUSH_ARG        #200
x   CALL            %3, %2
x   OUTPUT          %3          ; This might call additional formatters based on the selected output

x   JMP             <byte after foo>
x   STORE_NAME      %0, "foo()"
x   PUSH_ARG        #1
x   CREATE_ANNT     %1, "start"
x   ANNOTATE        %0, %1
x   STORE_CONST_ATTR  #201, %0, "__doc"
x   STORE_ADDR      %0, <next bc>
x   PUSH_FRAME
x   RETURN          #105
```

### If

```cpp
a = 6
if(a < 2) {
    a = 2
}
else a = 0
```

```
x   STORE_NAME      %0, "a"
x   STORE_CONST     %0, #6

x   LT              %1, %0, #2
x   JMP_IF_FALSE    %1, <bc of else>
x   STORE_CONST     %0, #2
x   JMP             <bc after else>
x   STORE_CONST     %0, #0
```

### Switch

```go
switch(a) {
case 1, 2: return 4
case 3: return 0
default: return -1
}
```

```
x       BUILD_LIST      %1
x       LIST_PUSH_CONST %1, #1
x       LIST_PUSH_CONST %1, #2
x       LIST_PUSH_CONST %1, #3
x       BUILD_LIST      %0
x       LIST_PUSH_ADDR  %1, add1
x       LIST_PUSH_ADDR  %1, add1
x       LIST_PUSH_ADDR  %1, add2
x       LOAD_NAME       %3, "a"

x       SWITCH %3, %0, %1, addr3
add1    RETURN_CONST #4
add2    RETURN_CONST #0
add3    RETURN_CONST #200
```

### Enum

```cpp
enum Colors {
    BLACK
    WHITE
}
```

```
x       BUILD_LIST      %0
x       STORE_STR_CONST #1, "BLACK"
x       LIST_PUSH_CONST %0, #1
x       STORE_STR_CONST #2, "WHITE"
x       LIST_PUSH_CONST %0, #2
x       BUILD_ENUM      %1, %0, "Colors"
```

### Scopes

```cpp
Foo.goo
```

```
x   LOAD_NAME %0, "Foo"
x   LOAD_ATTR %1, %0, "goo"
x   OUTPUT %1
```

### Try, catch, finally and raise

```cpp
try {
    "Hi"
    raise Exception("oups")
} catch (e:NameError) {
    e
} catch (e) {
    "oh no"
} finally {
    "done"
}
```

```
x   CATCH_TYPED  "e", "NameError", <addr of catch e:NameError>
x   CATCH  "e", <addr of catch e:NameError>

; try
x   STORE_STR_CONST #0, "Hi"
x   STORE_CONST     %0, #0
x   OUTPUT %0
x   ... ; loading Exception
x   RAISE  %100
x   JMP <addr of finally>

; catch e:NameError
x   LOAD_NAME %1, "e"
x   OUTPUT %1
x   JMP <addr of finally>

; catch e
x   STORE_STR_CONST #1, "oh no"
x   STORE_CONST     %2, #1
x   OUTPUT %2
x   JMP <addr of finally>

; Finally
x   STORE_STR_CONST #2, "done"
x   STORE_CONST     %3, #2
x   OUTPUT %3 

x   POP_CATCH
```

### Spaces

```cpp
space Foo {
    NAME = "Foo"

    space {

    }
}
```

```
x   BUILD_SPACE %0, "Foo"
x   STORE_STR_CONST #0, "Foo"
x   STORE_CONST %0, #1
x   STORE_NAME  %0, "NAME"

x   BUILD_SPACE %1 "0s"
x   POP_FRAME

x   POP_FRAME
```

### For loops

```cpp
for (i: lst) {
    i
}
```

```
x   SET_NAME    %0, "i"
x   LOAD_NAME   %1, "lst"
x   RESET_ITER  %1
4   FOR         %0, %1, <addr_after for>
x   LOAD_NAME   %2, "i"
x   OUTPUT      %2
x   JMP         4
```

### Break and Continue

```cpp
while(true) {
    break
    continue
}
```

```
x       STORE_BOOL_CONST  #300, true
x       STORE_CONST  %30,  #300
x       JMP_IF_FALSE  %30, 4
x       JMP  <end of while>
x       JMP  <start of while or end of while-1>
x       JMP  1
x       END
```

### Short circuit and and or

```cpp
a || b
```

```
x       LOAD_NAME %0, "a"
x       JMP_IF_FALSE %0, <b>
x       STORE %1, %0
x       JMP <after b>
x       LOAD_NAME %2, "b"
x       STORE %1, %2
x       OUTPUT %1
```

### List comprehension

```py
c = "Hello there programmer"
f = [",", "_", ""]
[c if(c != " ") else f : c = greet, f = separs]
```

```java
for (c: greet) {
    for (f: separs) {
        if (c == " ")
            lst += [f]
        else
            lst += c
    }
}
```
