r = rand_int(-10, 10)
assert(r >= -10 and r <= 10, "Generated outside of min and max")
r = rand_int(0,0)
assert(r == 0, "Generated not 0?")