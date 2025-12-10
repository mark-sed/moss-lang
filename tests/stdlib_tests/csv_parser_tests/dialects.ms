import csv_parser as csv

no_dq = csv.Dialects.Dialect(double_quote=false, escape_char="|", strict=true)

csv.list_reader(r"""
a,b,c
a,3,|"
d,3,"Peter ""Song"" "
""", dialect=no_dq)
"\n"

space_dial = csv.Dialects.Dialect(delimiter=" ")

csv.list_reader(r"""
name age year
peter 24 2025
"some guy" 50 2005
""", dialect=space_dial)
"\n"

csv.list_reader("""a,b,c;;hello
there,some,value;;2,3,4""", csv.Dialects.Dialect(line_terminator=";;"))
"\n"