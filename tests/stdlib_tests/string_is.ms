" ".isspace()
"\n"
" a".isspace()
"\n"
" \f\n\r\t\v".isspace()
"\n"
"".isspace()
"\n"

"----\n"

"aščα".isalpha()
"\n"
"Č".islower()
"\n"
"Č".isupper()
"\n"

"----\n"

"aaaA".islower()
"\n"
("a").islower()
"\n"
"".islower()
"\n"
"\n".islower()
"\n"
"klmnščoicp".islower()
"\n"

"-----\n"

"AAAAa".isupper()
"\n"
"Z".isupper()
"\n"
"\n".isupper()
"\n"
"12".isupper()
"\n"
"ABCDE".isupper()
"\n"

"-----\n"

"AbCd".isalpha()
"\n"
"ab cd".isalpha()
"\n"
"aB2c".isalpha()
"\n"
"".isalpha()
"\n"
"M".isalpha()
"\n"

"-----\n"

"Abcd".isalnum()
"\n"
"\n".isalnum()
"\n"
"32".isalnum()
"\n"
"ab1c".isalnum()
"\n"
"12a 3".isalnum()
"\n"

"-----\n"