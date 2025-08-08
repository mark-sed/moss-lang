Generators.HTML.FOOTER = ""
Generators.HTML.STYLE_PATH = ""

md"""\
# Title
Some text after title **with bold** & some bad values \"<>\".
[also a link](link.com) is a _link_.

* Table value 1
    * Inner value 2
* Table value 2

1. Numbered list value
2. Numbered value list 2
    * Inner non numbered value
    * An another one
3. Last numbered value

> Quote
>> And inner quote

_Here_ is ~~a~~ `code` block:
```cpp
for (l: parsed) {
    l.to_txt()
}
```

## Title 2 ##

![A cat](https://example.com/cat.jpg "Cute cat")

This is *italic* text, and this is **bold** text.

You can also _underline_ (using HTML: `<u>text</u>`).

This is **bold and _nested italic_** text.

This is `inline code`.

The END.
"""
