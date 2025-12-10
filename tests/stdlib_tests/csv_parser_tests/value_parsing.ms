import csv_parser as csv

// New lines are skipped
v_quotes = csv.list_reader("""\


id,name,age,country_code
42,"Marek",29,CZ
22,Jon,"55",US


""")

v_no_quotes = csv.list_reader("""\
id,name,age,country_code
42,Marek,29,CZ
22,Jon,55,US""", csv.Dialects.DEFAULT)

v_quotes == v_no_quotes
"\n"

csv.list_reader("""\
id,name,age,country_code
42,"Gabriel ""Gabi""\",29,CZ
22,Jon "Snow","55",US
0,"Lorem
Ipsum",0,NA
""")
"\n"

csv.list_reader("""


""")
"\n"

csv.list_reader("""
a, b, c
,,

hi,there,maam


1,2,3

""")
"\n"

csv.list_reader("""a\n1\n2\n3""")
"\n"

csv.list_reader("""a;1;2;3;4""", csv.Dialects.Dialect(delimiter=";"))
"\n"