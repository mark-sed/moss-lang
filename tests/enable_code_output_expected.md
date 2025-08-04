# Using code output
When enable_code_output is set then all the code
will be outputted as well.
```moss
fun foo() {
    /// @returns string
    return "hi\n"
}
```
```moss
foo()
```
_[Output]:_
```
hi
```
## Here the code output was disabled
We get some value but it is not guarded as [Output]
2
