# Notes

"Note" is a term that refers to any expression that will be outputted
to the output file ("document"). The document and its format is chosen
by the user then running the interpreter and therefore it might not be
even created or it can even be the standard output.

Any right hand side expression is considered a note. The most common and simple
one is just a string literal.

```py
"This will be in document"

"""
This multiline
string as well.
"""
```

But it can be also things like function calls. In Moss any value has a to string
converting function, and that will be always called on a non-string note value.

```cpp
fun foo() {
    return 42
}

foo() // 42 will be in the output file
```

```cpp
class MyClass {
    // Code
}

MyClass() // Will be converted to string and outputted
```

If one wants to call a function and disregard the return value, the
"silent" operator `~` can be used.

```cpp
~foo() // 42 will not be in the output file
```

> Note: For one-liners, using notes as prints is quite suitable and saves
> calls to `print`.

## xString notes

xString stands for a string that is prefixed by some identificator. There are
built-in formats, such as markdown (`md`) or `txt`. But there can be also
custom prefixes, that are needed for outputting custom format. If this prefix
matches any global function name that has been annotated as `@formatter` and
this function takes one argument, then this string is also sent to it for
additional parsing (if this is desired).


```cpp
class EquationNote : Note {
    COUNT = 0

    fun EquationNote(s) {
        COUNT += 1
        this.count = COUNT
        super(s)
    }
}

@formatter
fun eq(s) {
    return EquationNote(s) 
}

eq"""
x_1 = ln_{22}(y) + 4 
"""
```

Such values act the same as normal strings and can be assigned and used in
expressions

```cpp
a = md"# Header"
```

The actual output extends `String` type, but is of type `Note` and more
specifically of type `MarkdownNote`. So in the given example with function `eq`
the type of `a` is `Note`, which might be used for some additional analysis.

## Formatters

As mentioned in previous section there can be specialized formatter functions
which match the xString prefix and are called with this string for additional
formatting. Their return value is then used for the output. This syntax is
intended for notes and even though it can be used for any function that takes
string arguments, it is not recommended as it might make the code way less
readable and what people expect from xStrings are Note objects.

Keep in mind that formatters are not obligatory and you can have your own
xString without formatter and even a converter (in that case it is worked with
as with a normal note that stores the string value and prefix).

```cpp
@formatter
fun chess(s) {
    return s.replace("K", "♔").replace("Q", "♕").replace("R", "♖").replace("B", "♗").replace("N", "♘").replace("p", "♙")
}

chess"""
1.e4 Nf6 2.e5 Nd5 3.d4 d6 4.Nf3 g6 5.Bc4 Nb6 6.Bb3 Bg7 7.Qe2
Nc6 8.O-O O-O 9.h3 a5 10.a4 dxe5 11.dxe5 Nd4 12.Nxd4 Qxd4
13.Re1 e6 14.Nd2 Nd5 15.Nf3 Qc5 16.Qe4 Qb4 17.Bc4 Nb6 18.b3
Nxc4 19.bxc4 Re8 20.Rd1 Qc5 21.Qh4 b6 22.Be3 Qc6 23.Bh6 Bh8
24.Rd8 Bb7 25.Rad1 Bg7 26.R8d7 Rf8 27.Bxg7 Kxg7 28.R1d4 Rae8
29.Qf6+ Kg8 30.h4 h5 31.Kh2 Rc8 32.Kg3 Rce8 33.Kf4 Bc8 34.Kg5
1-0
"""
```
_[Output]:_
```
1.e4 ♘f6 2.e5 ♘d5 3.d4 d6 4.♘f3 g6 5.♗c4 ♘b6 6.♗b3 ♗g7 7.♕e2
♘c6 8.O-O O-O 9.h3 a5 10.a4 dxe5 11.dxe5 ♘d4 12.♘xd4 ♕xd4
13.♖e1 e6 14.♘d2 ♘d5 15.♘f3 ♕c5 16.♕e4 ♕b4 17.♗c4 ♘b6 18.b3
♘xc4 19.bxc4 ♖e8 20.♖d1 ♕c5 21.♕h4 b6 22.♗e3 ♕c6 23.♗h6 ♗h8
24.♖d8 ♗b7 25.♖ad1 ♗g7 26.♖8d7 ♖f8 27.♗xg7 ♔xg7 28.♖1d4 ♖ae8
29.♕f6+ ♔g8 30.h4 h5 31.♔h2 ♖c8 32.♔g3 ♖ce8 33.♔f4 ♗c8 34.♔g5
1-0
```

In this example the English localized chess piece names in the notation were
replaces with their UNICODE equivalent. As most people will probably find it
lot more difficult to write emojis rather than letters this makes it lot
simpler to write the notation. 

You can also imagine lot more complex formatter which could generate the output
based on some interpreter output flag (using `out` module) and then maybe
localize it to some other language or anything of that kind.

> __Note__: Formatters are not meant to work like converters. If you write
> a note in your custom markup and want it to be convertible to others, don't
> just output Markdown note in the formatter, rather output your custom Note and
> then write a converter which generates the Markdown.

### fString and rStrings

fStrings (formatted strings with interpolation) and rStrings (raw strings -
strings that don't accept escape sequences) on the outside look like xStrings
and Moss works with them in that way, but they are a special kind. It is not
possible to have a formatter which would work for fString. So fString and
rString are handled internally. So you cannot just call their formatters
as you could with custom ones. 

Calling multiple formatters on each others output does not make sense as this
can be handled by converters. Only case where this makes sense are calling them
on fStrings and rStrings. In such cases one must use the explicit function call.

```cpp
md(f"""
# {title}

__Date__: {get_localized_date()}

__Name__: {get_name()}
""")
```

## Non-literal notes

Sometimes you might want to output a value returned by a function and it
might be in some format (not just plain string). You can just pass this
value to the converter function.

```cpp
fun header(s) = "# " ++ s

md(header("Introduction"))
```

## Output formats

There are predefined output formats, which you can always use, such as
`md` (markdown) or `txt`.

You can also add a custom output format and use one of the existing
for your transformation.

```cpp
import out

@converter(out.Formats.Markdown, ["tx1", "text1", "txt1"])
fun tx1_from_md(s, ... flags) {
    return s.replace("###", "<h3>").replace("##", "<h2>").replace("#", "<h1>")
}

md"""
# Introduction

This is your introduction to moss notes.
"""
```

In the example above function `tx1_from_md` was annotated as a converter for
output type `tx1` (and other forms of it) and that it accepts type `md`.
If the user selects `tx1` as the output format, then all notes will be
converted to markdown and then passed to `tx1_from_md` and its output used.

> __Note__: Keep in mind that some formats loose information and so it might
> be better to use some existing high information format for your conversion.

Moss tries to find the best conversion function or pipeline to use. In the
example before with custom note type `eq`, there was no conversion function
and therefore this equation would always end-up in the output as a plain-text
one.

For example if the user were to try to generate `pdf` output, Moss would look
at all existing converters of notes with type `eq`, there would be none
and therefore it would choose to use it as is (plain text) and then pass
this to converter from plain text to `tex` and then this one to `pdf`
(pdf is compiled from tex sources, more about this later).

So if we want to keep the format of our type `eq` we need to provide some
converter that could be then used to convert to a type, which converts to
desired output format.

```cpp
import out

@converter("eq", out.Formats.LaTeX)
fun eq2tex(s) {
    return r"\begin{equation} \tag{" ++ s.count ++ "}\n" ++ s ++ r"\n\end{equation}"
}
```

With this converter we can now even generate markdown output and this function
will be used to generate `tex` note and then converter from `tex` to `markdown`
will be used and `eq` will end up looking probably something like this:

`` `x_1 = ln_{22}(y) + 4` ``

Markdown does not have nearly as many formatting options as LaTeX has and so
lots of information, like the equation count is lost and if we want to keep
this information it might be needed to add converter for these types as well.

### Formatting pipeline

The formatting pipeline is chosen based on the existing converters,
their order of definitions and the pipeline length.

```cpp
import out

@converter("eq", out.Formats.LaTeX)
fun eq2tex(s) {
    return r"\begin{equation} \tag{" ++ s.count ++ "}\n" ++ s ++ r"\n\end{equation}"
}

@converter("eq", out.Formats.Markdown)
fun eq2md(s) {
    return f"Eq. {s.count}: `" ++ s ++ "`"
}
```

In the example above if output format is `tex`, then the first function will be
used. If the output format is `md`, then the second one will be used. If the
output format will be `txt`, then the closest defined format in Moss to `txt`
is Markdown, so the second function will be used and its output will be passed
to converter from `md` to `txt`. If the second function did not exist, then
the first one would be used, the `tex` note will be then converted to `md`,
which will be then converted to `txt`.

> __Note__: When running scripts in terminal, the output is not actually
> plain-text, but its `term` (`Terminal`) format. This might be used for
> instance to format to 80 characters or use colors.

## Output file creation

Some output formats are not text formats, but might still be desired output
(such as `pdf`). These formats use generators. These generators modify the
environment and don't have to return any value.

Generators are called after all notes have been collected (script terminated).

```cpp
import out
import sys

@generator(out.Formats.PDF)
@platform(sys.Platform.Linux) // Invoked only on Linux
fun tex2pdf(s) {
    tmp = out.create_tmp(s)
    sys.system(f"pdflatex {tmp} -o {out.out_path()}")
}
```

Let's say that we can still convert generator output, then we should modify the
output from the generator to return some custom value that can be used to
generate more formats.

```cpp
import out
import sys

@generator(out.Formats.PDF)
@platform(sys.Platform.Linux) // Invoked only on Linux
fun tex2pdf(s) {
    tmp = out.create_out_tmp(s)
    // Here we use out_path() which returns path to the output file
    // that the user has chosen, but if this is not the final generator
    // in the pipeline, then this will be some dummy value
    sys.system(f"pdflatex {tmp} -o {out.out_path()}")
    return out.out_path()
}

@generator("pdfa")
@platform(sys.Platform.Linux)
fun pdf2pdfa(path) {
    // Here out_path will be the actual one chosen by the user
    sys.system(f"bash pdf2pdfa.sh {path} {out.out_path()}")
}
```

With the code above if we create script with some `eq` notes, then
all these notes, once encountered, will be passed to the converter to `tex` and
once the script terminates this output will be passed to the `tex2pdf` and then
this output to `pdf2pdfa`.

> __Note__: Be careful when using pipelined generators as they don't
> work with Notes and it might be difficult to maintain multiple pipelinings.
> It might be better to create direct generators from convertors (rather than
> generators from generators).

## Notebook

Moss also allows for notebook output (interpreter option), which will
generate the output also with the accompanying code and its output.

```cpp
@!notebook
// Comments are not part of the output file

md"""
This text will be in the output file.
"""

fun foo() = 42

print(foo())
```

The script above if outputted as Markdown will look as following: "

This text will be in the output file.

```cpp
fun foo() = 42

print("Hello, " ++ foo() ++ "!")
```
_[Output]:_
```
Hello, 42!
```
"

This notebook style can be used to showcase code, its output and annotate it
with formatted notes.

Notebook will be generated only from the main script and therefore should be
written as a single file script (if all code is to be shown). If some other
called script is outputting some values and notes, these will end up in the
output section.

The code snippets (code until the next note) will be passed to convertors
with the type `code` (`Code`) and its output as `codeout` (`CodeOutput`).
The code output is encapsulated in `CodeOutputNote` class which contains
`stdout` and `stderr`, if these 2 are needed to be parsed separately.

## Documentation

Functions, classes, space variables, enums and spaces can contain documentation
notes `d` (`Documentation`), which can be accessed at runtime but also used
to generate documentation.

```cpp
fun divxy(x:Int, y:Int) {
    d"""
    Divides x by y.
    @param x Numerator
    @param y Denominator
    @returns x divided by y
    @warning this fails if y is 0
    """
    return x / y
}
```

Documentation is special kind of a note as it is also added as an attribute
to Moss constructs and can be accessed with `help(x)` function or `__doc`
attribute.

There is a Documentation output mode, which does not execute any code nor notes
and sends declarations into special `doc_writer` convertors/generators.

```cpp
@doc_writer(out.Formats.HTML)
fun doc2html(code) {
    // Conversion code
}
```

Documentation generators are very specialized functions that probably want to
be created only by documentation generation tools or analysis tools.

## Using external converters

Having to write/copy convertors to every program is of course not the best
approach. One can simply import a module, which provides these formats
and convertors and those will be used. If some are not to one's liking, those
can be implemented or imported from different module. These converters need to
be imported into the global scope to overshadow the existing moss converters.
Alternatively it is possible to just call desired converter from custom
overshadowing one.

The latest defined scope accessible converter will be used.

```cpp
import BestConverters.*
import ShinyMarkdown.md2tex

@converter("fasta", "fastq")
fun fasta2fastq(f) {
    import FastaConv
    return FastaConv.to_fastq(f)
}

md"""
# Intro

Prague is the capital of Czech Republic.
"""
```

## Unified converter approach

In many cases you might not need any converters at all if the ones provided by
Moss are good enough, but if there is some format you might use in all of your
scripts, then instead of just copy-pasting the same convertor into all scripts
or importing it into all of them, even some older ones, you might want to
specify the convertors when running the scripts. This allows also for change
of convertors without changing any code. You can simply specify the module that
should be used with highest priority for conversion to the interpreter using
`-c` or `--converters` options.

```
moss -c my_converters.ms main.ms
```