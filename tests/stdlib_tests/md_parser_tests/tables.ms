import md_parser.*

txt = """\
# Title

|A|B|C|D|
|1| |0|3|
| |3|3|0|
|a|b|0| |

| T1 | T2 |

| Item              | In Stock | Price |
| :---------------- | :------: | ----: |
| Someone           |   True   | 23.99 |
| Other 24          |   True   | 23.99 |
| Coders            |  False   | 19.99 |
| Non coders        |  False   | 42.99 |

| Month | Savings |
| -------- | ------- |
| January | $250 |
| February | $80 |
| March | $420 |
"""

parsed = parse_md(txt)

for (l: parsed) {
    l.type++"\n"
}

for (l: parsed) {
    l.to_txt()++"\n"
}