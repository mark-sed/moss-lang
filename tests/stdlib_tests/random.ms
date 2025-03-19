r = rand_int(-10, 10)
assert(r >= -10 and r <= 10, "Generated outside of min and max")
r = rand_int(0,0)
assert(r == 0, "Generated not 0?")

for(i: 0..10) {
    rf = rand_float(-100, 100)
    assert(rf >= -100 and rf <= 100, "Generated float outside of min and max")
}
assert(rand_float(10.0, 10.0) == 10.0, "Generated not 10.0?")

rf2 = rand_float()
assert(rf2 >= 0.0 and rf2 <= 1.0, "rand_float not in [0, 1]")