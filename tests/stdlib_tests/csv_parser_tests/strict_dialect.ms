import csv_parser as csv

strict = csv.Dialects.Dialect(strict=true)

try {
    csv.list_reader("""


a,b,c
"a",3,"c"
"d,3,5
""", dialect=strict)
} catch (e:csv.CSVError) {
    e
}

try {
    csv.list_reader("""
a,b,c
1,2,3
4,5
6,7,8
""", dialect=strict)
} catch (e:csv.CSVError) {
    e
}

try
    csv.list_reader("""a;;1;;2;;3;;4""", csv.Dialects.Dialect(delimiter=";;"))
catch(e:csv.CSVError)
    e

try
    csv.list_reader("""a\n1""", csv.Dialects.Dialect(quote_char=""))
catch(e:csv.CSVError)
    e

try
    csv.list_reader("""a\n1""", csv.Dialects.Dialect(escape_char="?!?!"))
catch(e:csv.CSVError)
    e

// Line terminator can be multiple chars
~csv.Dialects.Dialect(line_terminator="\r\n")