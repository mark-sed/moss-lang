import csv_parser as csv

val = """\
"id","name","age","country_code"
42,"Marek",29,"CZ"
22,"Jon",55,"US"
"""

csv.list_reader(val, csv.Dialects.Dialect(quoting=csv.Dialects.Quoting.NON_NUMERIC))
"\n"