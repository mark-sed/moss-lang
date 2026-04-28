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

js.dict_reader("""
{
    "a": 42,
    "b": -0.0001,
    "c": -200,
    "d": 20e-7
}
""")
"\n"

js.dict_reader("""
{"a": "00",
    "a12": 23,
    "b1": ",",
    "b45": 0.00}
""")
"\n"