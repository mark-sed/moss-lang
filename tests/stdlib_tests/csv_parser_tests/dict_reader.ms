import csv_parser as csv

f = open(args[0])
csv.dict_reader(f) == [{"Name": "Peter", "Year": "2000", "Age": "24"}, {"Name": "Jon", "Year": "2001", "Age": "4"}, {"Name": "Ivan", "Year": "1998", "Age": "42"}, {"Name": "Ed", "Year": "1700", "Age": ""}]
"\n"

~f.close()

csv.dict_reader("""
1,2,3,4
5,6,7,8
9,10,11,12
""", ["num1", "num2", "num3", "num4"]) == [{"num2": "2", "num3": "3", "num1": "1", "num4": "4"}, {"num2": "6", "num3": "7", "num1": "5", "num4": "8"}, {"num2": "10", "num3": "11", "num1": "9", "num4": "12"}]
"\n"

csv.dict_reader("")
"\n"