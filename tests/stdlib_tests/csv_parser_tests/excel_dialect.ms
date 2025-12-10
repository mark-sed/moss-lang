import csv_parser as csv
with_open(args[0], fun(f) = csv.list_reader(f, csv.Dialects.EXCEL))
"\n"
with_open(args[1], fun(f) = csv.list_reader(f, csv.Dialects.EXCEL_TAB))
"\n"