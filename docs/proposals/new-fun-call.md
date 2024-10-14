# New function bytecode

Function needs a value of its own and store the argument info, such as
types and values. On top of that it needs to match types based on actual
Value (type) not string name since then calling into other modules would
cause issues.

Because of this new bytecodes need to be introduced.

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

; This will create FunValue %2 will store "foo"
; all the other stuff will be parsed into this value as well
CREATE_FUN      %2, "foo" "a,b,c,d"  
SET_DEFAULT     %2, 1, %0
SET_TYPE        %2, 1, "Int"
SET_VARARG      %2, 2
SET_DEFAULT     %2, 3, %1
FUN_BEGIN       %2
; Body
STORE_CONST_NIL #2
RETURN_CONST    #2

CREATE_FUN      %3, "foo(a)"

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

# Bad fun(s)

```cpp
fun test1(a, b=3, c) {

}

fun test2(a, a) {

}

fun test3(... c, d) {

}


// Bad call
fun test4(a, b) {

}

fun test4(c) {

}

test4(a=4)
```