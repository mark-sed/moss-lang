# Fstrings

FStrings need support in scanner or parser, which then calls scanner on
the contents of it.

It might not be the fastets to create new scanner while parsing, so it will
be better to call string parsing when `f"` is found, but if `{` is found, then
start the tokenizer again and collect tokens up until `}` and after `"` is
reached, then return a new fString token with list of lists of tokens
which will be indexed into the string in parser.

Parser then has to output string concatenation expresion, with the
parts of the original string followed by the expressions in correct order.