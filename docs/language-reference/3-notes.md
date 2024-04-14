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

```py
fun foo() {
    return 42
}

foo() # 42 will be in the output file
```

```py
class MyClass {
    # Code
}

MyClass() # Will be converted to string and outputted
```

If one wants to call a function and disregard the return value, the
"silent" operator `~` can be used.

```py
~foo() # 42 will not be in the output file
```

> Note: For one-liners, using notes as prints is quite suitable and saves
> calls to `print`.

## xString notes

xString stands for a string that is prefixed by some identificator. There are
built-in formats, such as markdown (`md`) or `pdf`. But there can be also
custom prefixes, that are needed for outputting custom format. If this prefix
matches any global function name and this function takes one argument, then
this string is also sent to it for additional parsing (if this is desired).


```go
class EquationNote : Note {
    COUNT = 0

    new EquationNote(s) {
        COUNT += 1
        this.count = COUNT
        super(s)
    }
}

fun eq(s) {
    return EquationNote(s) 
}

eq"""
x_1 = ln_{22}(y) + 4 
"""
```

Such values act the same as normal strings and can be assigned and used in
expressions

```py
a = md"# Header"
```

The actual output extends `String` type, but is of type `Note` and more
specifically of type `MarkdownNote`. So in the given example with function `eq`
the type of `s` is `Note`, which might be used for some additional analysis.

## Non-literal notes

Sometimes you might want to output a value returned by a function and it
might be in some format (not just plain string). You can just pass this
value to the converter function.

```py
fun header(s) = "# " ++ s

md(header("Introduction"))
```

## Output formats

There are predefined output formats, which you can always use, such as
`md` (markdown) or `pdf`.

You can also add a custom output format and use one of the existing
for your transformation.

```py
import out

@converter(out::Formats::Markdown, ["tx1", "text1", "txt1"])
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

```py
import out

@converter("eq", out::Formats::LaTeX)
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

```py
import out

@converter("eq", out::Formats::LaTeX)
fun eq2tex(s) {
    return r"\begin{equation} \tag{" ++ s.count ++ "}\n" ++ s ++ r"\n\end{equation}"
}

@converter("eq", out::Formats::Markdown)
fun eq2tex(s) {
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

```py
import out
import sys

@generator(out::Formats::LaTeX, out::Formats::PDF)
@platform(sys::Platform::Linux) # Invoked only on Linux
fun tex2pdf(s) {
    tmp = out::create_tmp(s)
    sys.system(f"pdflatex {tmp} -o {out::out_path()}")
}
```

Let's say that we can still convert generator output, then we should modify the
output from the generator to return some custom value that can be used to
generate more formats.

```py
import out
import sys

@generator(out::Formats::LaTeX, out::Formats::PDF)
@platform(sys::Platform::Linux) # Invoked only on Linux
fun tex2pdf(s) {
    tmp = out::create_out_tmp(s)
    # Here we use out_path() which returns path to the output file
    # that the user has chosen, but if this is not the final generator
    # in the pipeline, then this will be some dummy value
    sys.system(f"pdflatex {tmp} -o {out::out_path()}")
    return out::out_path()
}

@generator(out::Formats::PDF, "pdfa")
@platform(sys::Platform::Linux)
fun pdf2pdfa(path) {
    # Here out_path will be the actual one chosen by the user
    sys.system(f"bash pdf2pdfa.sh {path} {out::out_path()}")
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

```py
@!notebook
# Comments are not part of the output file

md"""
This text will be in the output file.
"""

fun foo() = 42

print(foo())
```

The script above if outputted as Markdown will look as following: "

This text will be in the output file.

```
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
notes `doc` (`Documentation`), which can be accessed at runtime but also used
to generate documentation.

```py
fun divxy(x:Int, y:Int) {
    doc"""
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

```py
@doc_writer(out::Formats::HTML)
fun doc2html(code) {
    # Conversion code
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

```py
import BestConverters::*
import ShinyMarkdown::md2tex

@converter("fasta", "fastq")
fun fasta2fastq(f) {
    import FastaConv
    return FastaConv::to_fastq(f)
}

md"""
# Intro

Prague is the capital of Czech Republic.
"""
```
