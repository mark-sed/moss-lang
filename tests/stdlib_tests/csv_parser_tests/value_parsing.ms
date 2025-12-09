import csv_parser as csv

v_quotes = csv.list_reader("""\
id,name,age,country_code
42,"Marek",29,CZ
22,Jon,"55",US
""")

v_no_quotes = csv.list_reader("""\
id,name,age,country_code
42,Marek,29,CZ
22,Jon,55,US
""")

v_quotes == v_no_quotes
"\n"