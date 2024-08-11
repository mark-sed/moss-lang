# Moss error messages proposal

Error messages in interpreted languages aren't usually as detailed and
have its limitations. At the same time any runtime error can be caught in try
catch block and those have to be treated as exceptions.

Error messages should be colored (unless redirected), but not overly cluttered.

## What needs to be displayed

* File in which this error was found
* Line and column at which the error starts (possibly could be even range)
* Error message describing this
* Code snippet with indicator for the error range

## Examples

```
// Comment
assert()
```

```
moss: error: assert_file.ms:1:8:
    | assert expects 1 or 2 arguments -- condition and optional message.
    |
  2 | assert()
    | ~~~~~~~^
```