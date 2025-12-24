import csv_parser

cf1 = args[0]
cf2 = args[1]

f = open(cf1, "w")
writer = csv_parser.CSVWriter(f)
~writer.write_row(["Name", "Year", "Age"])
~writer.write_row(["Marty", "1990", "21"])
~writer.write_row(["Tom", "1999", "17"])
~writer.write_row(["Misha", "2012", "19"])
~f.close()

f2 = open(cf1)
read = csv_parser.list_reader(f2)
~f2.close()
read
"\n"
f = open(cf2, "w")
writer = csv_parser.CSVWriter(f)
~writer.write_rows(read)
~f.close()

c1 = with_open(cf1, fun(f)=f.read())
c2 = with_open(cf2, fun(f)=f.read())
c1 == c2
"\n"