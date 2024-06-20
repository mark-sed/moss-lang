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
- 0 - Invalid value
- 1 - 99 - int values 
- 100 - float 0.0
- 101 - float 1.0
- 102 - float +inf
- 103 - float -inf
- 104 - float nan
- 105 - nil
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

00h - END - End of code

01h - LOAD          %dst, "name"
02h - LOAD_ATTR     %dst, %src, "name"
03h - LOAD_GLOBAL   %dst, "name"
xxh - LOAD_NONLOC   %dst, "name"

xxh - STORE             %dst, %src
xxh - STORE_NAME        %dst, "name"
xxh - STORE_CONST       %dst, #val
xxh - STORE_ADDR        %dst, addr
xxh - STORE_ATTR        %src, %obj, "name"
xxh - STORE_ADDR_ATTR   addr, %obj, "name"
xxh - STORE_CONST_ATTR  #val, %obj, "name"

xxh - STORE_INT_CONST   #dst, int
xxh - STORE_FLOAT_CONST #dst, float
xxh - STORE_BOOL_CONST  #dst, bool
xxh - STORE_STR_CONST   #dst, "string"

xxh - JMP               addr
xxh - JMP_IF_TRUE       %src, addr
xxh - JMP_IF_FALSE      %src, addr
xxh - CALL              %dst, addr
xxh - RETURN            %val
xxh - RETURN_CONST      #val
xxh - RETURN_ADDR       addr
xxh - PUSH_ARG          %val
xxh - PUSH_CONST_ARG    #val
xxh - PUSH_ADDR_ARG     addr

xxh - IMPORT        %dst, "name"
xxh - IMPORT_ALL    "name"

xxh - PUSH_PARENT   %class
xxh - CREATE_OBJ    %dst, %class
xxh - PROMOTE_OBJ   %src, %class
xxh - BUILD_CLASS   %src
xxh - COPY          %dst, %src
xxh - DEEP_COPY     %dst, %src

xxh - CREATE_ANNT   %dst, "name"
xxh - ANNOTATE      %dst, %annot

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
xxh - XOR       %dst, %src1, %src2
xxh - SC_AND    %dst, %src1, %src2
xxh - SC_OR     %dst, %src1, %src2
xxh - SUBSC     %dst, %src, %index
xxh - SLICE     %dst, %src, %range

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
xxh - SC_AND2   %dst, #val, %src2
xxh - SC_OR2    %dst, #val, %src2
xxh - SUBSC2    %dst, #src, %index
xxh - SLICE2    %dst, #src, %range

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
xxh - SC_AND3   %dst, %src1, #val
xxh - SC_OR3    %dst, %src1, #val
xxh - SUBSC3    %dst, %src, #index

xxh - ASSERT    %src

xxh - COPY_ARGS

xxh - RAISE         %val
xxh - CHECK_CATCH   %dst, %class

xxh - LIST_PUSH         %val
xxh - LIST_PUSH_CONST   #val
xxh - LIST_PUSH_ADDR    addr
xxh - BUILD_LIST        %dst

xxh - BUILD_DICT        %keys, %vals

xxh - CREATE_RANGE      %dst, %start, %step, %end
xxh - CREATE_RANGE2     %dst, #start, %step, %end
xxh - CREATE_RANGE3     %dst, %start, #step, %end
xxh - CREATE_RANGE4     %dst, %start, %step, #end
xxh - CREATE_RANGE5     %dst, #start, #step, %end
xxh - CREATE_RANGE6     %dst, #start, %step, #end
xxh - CREATE_RANGE7     %dst, %start, #step, #end
xxh - CREATE_RANGE8     %dst, #start, #step, #end

xxh - SWITCH    %listvals, %listaddr, addr_def
xxh - FOR       %i, %iterator

## Examples

```py
import systemx as sx

a = sx.a + 430
b = 8
```

```
0   STORE_INT_CONST #200, 430
1   IMPORT      %0(sx), "systemx"

3   LOAD_ATTR   %3, %0(sx), "a"
3   ADD         %1(a), %3, #200

4   STORE_CONST %2(b), #8
```

When interpreted, STORE_CONST will be skipped as the vm will load
these values during parsing.

### Function

```cpp
fun foo(a: [Int, Float]) {
    print(a)
}

fun foo(a: SpecialString, b) {
    print(a)
    return b
}

~foo(11)
a = foo("hi")
```

```
0   STORE_STR_CONST #200, "hi"

x   STORE_NAME  %4, "foo(Int)"
x   ALIAS       %4, "foo(Float)"

x   JMP         <byte at which main (call to foo) starts>
x   STORE_ADDR  %4, <address of next bytecode>
x   PUSH_FRAME
x   LOAD        %1, "print"
x   CALL        %42, %1
x   RETURN      #105 (nil)

x   STORE_NAME  %5, "foo(SpecialString,_)"
x   ALIAS       %5, "foo(String,_)"

x   JMP         <byte at which main (call to foo) starts>
x   STORE_ADDR  %5, <address of next bytecode>
x   PUSH_FRAME
x   LOAD        %2, "print"
x   CALL        %42, %2
x   RETURN      %1

x   LOAD            %0, "foo"   ; Stores "fun list"[%4, %5] into %0
x   PUSH_ARG        #11
x   CALL            %1, %0      ; Goes through fun list and tries to match names pushed arg type names to stored names in fun list, will match to foo(Int) = %4
x   LOAD            %2, "foo"
x   PUSH_ARG        #200
x   CALL            %3, %2
x   STORE_NAME      %3, "a"     ; Global value, the name has to be stored
```

### Classes

```cpp
class MyClass : XClass {

    NAME = "My Class"

    new MyClass(a) {
        this.a = a
    }

    fun get_a() = this.a
}

myc = MyClass(42)
print(myc.get_a())
```

```
x   STORE_STR_CONST #200, "My Class"
x   STORE_NAME  %0, "MyClass"

x   JMP         <byte at which MyClass ends>
x   LOAD        %1, "XClass"
x   PUSH_PARENT %1
x   BUILD_CLASS %0

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
x   PROMOTE_OBJ %2, %3
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

```
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

```
switch(a) {
case 1, 2: return 4
case 3: return 0
default: return -1
}
```

```
x       LIST_PUSH_CONST #1
x       LIST_PUSH_CONST #2
x       LIST_PUSH_CONST #3
x       BUILD_LIST      %0
x       LIST_PUSH_ADDR  add1
x       LIST_PUSH_ADDR  add1
x       LIST_PUSH_ADDR  add2
x       BUILD_LIST      %1

x       SWITCH %0, %1, addr3
add1    RETURN_CONST #4
add2    RETURN_CONST #0
add3    RETURN_CONST #200
```

### Exception