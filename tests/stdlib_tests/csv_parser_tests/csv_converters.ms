import csv_parser

String(csv_parser.CSVParser("""
A,B,C,D
1,2,3,4
5,6,7,8
""").to_md())

String(csv_parser.CSVParser("""
A,B,C,D
1,2,3,4
5,6,7,8
""").to_html())
"\n"