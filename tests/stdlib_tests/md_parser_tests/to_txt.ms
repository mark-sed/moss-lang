import md_parser.*

txt = """\
# Title
Some text after title **with bold**.
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

|A|B| C |
|1|2| 3 |
|0.5|1.0|3|

The END.
"""

parsed = parse_md(txt)

for (l: parsed) {
    l.to_txt()++"\n"
}