# Moss Read-Eval-Print-Loop

REPL should be very straight forward and allow for basic scripting.

Later one there could be a better interactive version with possibly GUI and such
for easy use of moss.

REPL should be using colored output.

## Information displayed on startup

* Version
* Logo or name
* Note to use "help()" for help

## REPL specific command

REPL should accept command that are not part of the moss language. These
commands can be used to work with the REPL environment.

Note that `help` is also a useful REPL function, but it is a part of the moss
language and returns the doc string of an object.

* `exit()` - Exists REPL (same as ctrl-D on UNIX or ctrl-Z on Windows).
* `license()` - Prints license info.
* `credits()` - Prints project credits and useful links.

## REPL behavior

On startup repl displays information mentioned above and indicates waiting for
an input using the mode name followed by `> `.

Once a line is written and confirmed moss parses and interprets this line,
unless the line is not complete, then it waits for more lines indicated by
no mode prefix and tabbing out to the start of the construct. If more than
3 new empty lines are inputted then this is taken as a terminator (to make
sure user does not get stuck).

```
moss> if (a > 4)
      {
        print("hi")
      }
hi
moss>
```

Once a declaration is finished then it is evaluated (interpreted) and if it is
an empty expression then it value is also printed. This value is now stored also
in variable `_`. If there was an error than this error is printed and REPL
continues (this error is not saved into `_`).

```
moss> 42 * 2
84
moss> _ + 1
85
```

In case of multiple declarations, REPL prints only the last result.
