import json_parser as js

f = open(args[0])
readjs = js.dict_reader(f)
readjs["meta"]["version"] //3
"\n"
readjs["users"][0]["roles"][1] //"editor"
"\n"
readjs["users"][1]["profile"]["address"]["postal_code"] //"60200"
"\n"
readjs["events"][1]["items"][1]["price"] //15.5
"\n"
readjs["users"][0]["sessions"][1]["expires"] //nil
"\n"