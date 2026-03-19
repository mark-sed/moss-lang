@!enable_code_output
md"""
# Calculating π using Monte Carlo method

If we inscribe a circle into a square we can determine π from their areas.

If we have a square of side length `a`, then the area will be `a²` and inscribed
circle will have area of `π(d/2)²`. The ratio of these 2 areas will give us
`π/4`.

Here is the equation written in code:
"""
d = 100
(((Math.pi * (d/2) ^ 2) / (d * d)) * 4)

md"""
In this example getting π is not so great since we already need it to
calculate the area of the inscribed circle, but we can get the area or rather
the ratio using random number generator with uniform distribution.

If we generate points in the area of the square the point can either be placed
inside of the inscribed circle or outside. Since the distribution of the point
placements is uniform in infinite time we should get the exact ratio of dots
inside of the circle divided by the total amount of dots to be the same ratio
as above, which is π. The more dots we generate the more precise the π
estimation should be.
"""

fun generate_points(d:Int, amount:Int) {
    r = d/2
    inner = 0 // Counter of how many points were in the circle area.
    for (i: 0..amount) {
        // Generate [x, y] of a random point.
        x = rand_float(0, d)
        y = rand_float(0, d)

        // Check if the point is inside of the circle, by calculating its
        // distance from the center and it being <= r.
        // We get this value using Pythagoras' formula but just offset the
        // coordinates by the square center.
        distance = Math.sqrt((x - r) ^ 2 + (y - r) ^ 2)
        if (distance <= r)
            inner += 1
    }
    return inner
}

total_points = 100
inner_points = generate_points(d, total_points)

md"""
We have generated all our points and extracted the amount that has landed
in the inscribed circle:
"""

inner_points

md"""
So if we now divide this number by the total amount of points generated and
multiply it by 4 we get:
"""

"π ≈ " ++ (Float(inner_points) / total_points) * 4


md"""
The more samples we take the more precise result we should get so let's
get 1000x more samples now:
"""

total_points = 100000
inner_points = generate_points(d, total_points)
"π ≈ " ++ (Float(inner_points) / (total_points)) * 4