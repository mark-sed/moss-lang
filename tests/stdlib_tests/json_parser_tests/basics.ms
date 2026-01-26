import json_parser as js

js.dict_reader("""

  
  
""")

"\n"
js.dict_reader("""
{  

}
""")
"\n"

try {
    js.dict_reader("""
{\"""") 
} catch(e: js.JSONParsingError) {
    "caught\n"
}

try {
    js.dict_reader(r"""
    {
        "hey\g" :
    }
    """)
} catch(e: js.JSONParsingError) {
    "caught\n"
}

try {
    js.dict_reader(r"""
    {
        "name
        ": 42
    }
    """)
} catch(e: js.JSONParsingError) {
    "caught\n"
}