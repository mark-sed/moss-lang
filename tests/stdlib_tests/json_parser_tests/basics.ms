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

import json_parser as js

js.dict_reader("""
{
    "obj": {
        "a": 4,
        "b": 5
    },
    "obj2": {}
}
""")
"\n"

js.dict_reader("""
{
    "obj": {
        "a": [3, {"b": 3.4}, 0.0, [], "a"]
    },
    "arr": [1, 2, 3.3, [2, "hello"]]
}
""")
"\n"

try {
    js.dict_reader("""
    {"arr": [1, 2, 3.3, [2, "hello"]}
    """)
} catch (e:js.JSONParsingError) {
    "caught\n"
}

try {
    js.dict_reader("""
    {"arr": [1, 2 44]}
    """)
} catch (e:js.JSONParsingError) {
    "caught\n"
}