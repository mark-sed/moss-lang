import nested_finally as nf

"---\n"
~nf.raise_value(4.2)
~nf.raise_value("")
~nf.raise_value("i", early_exit=true)
~nf.no_raise()

try
    ~nf.raise_value(-8)
catch (e) {
    "caught in other mod\n"
} finally {
    "finally in other module\n"
}