# Notes generation

```py
md"""# Title
Some text.
"""
```

When this is outputted as plain text, then moss:
* Looks for function `md`, if it exists it is called.
* Returned value is saved as NoteValue with the prefix `md` as well.
* When OUTPUT encounters NoteValue it checks the user selected out format and
  the format of the NoteValue. If it matches it is outputted as is, if not, then
  Moss looks for `@converter` which takes `md` and outputs `txt`.
* If converter is found, then it is called and the output is printed into
  selected file.
* If generator is found, then the NoteValue is stored into a List of NoteValues
  for the given format.
* Once the script is terminated or function `output_notes()` is called, then
  Moss passes this List to the generator and if there is a pipeline of these,
  then its value is passed to another generator.

Bytecode generated will be the same as for any other call, but uses
CALL_FORMATTER opcode.

## Note from non-strings

In case of non-string the formatter function can be just normally called
and it is expected to return NoteValue with correct prefix and then it
works as the string note.

The same goes for standalone expressions. These are parsed into strings
and then from them into NoteValue with prefix 'txt'.
