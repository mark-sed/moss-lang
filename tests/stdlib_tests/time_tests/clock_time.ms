import time

some_august_day = time.StructTime(2025, 8, 5, 16, 55, 36, 1, 217, 1)
some_august_day ++ "\n"

lt = time.localtime(1754407575.7663472)
lt.tm_year++"-"++lt.tm_mon++"-"++lt.tm_mday++"_"++lt.tm_hour++":"++lt.tm_min++":"++lt.tm_sec
"\n"

curr_local_time = time.localtime()
curr_local_time.tm_year > 2024

"\n"
now = time.strftime("%Y-%m-%d %H:%M:%S")
now.length() > 0
"\n"

time.strftime("On some august day it was: %Y-%m-%d %H:%M:%S\n", some_august_day)
