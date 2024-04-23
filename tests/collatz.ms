md"""
# Collatz Conjecture

The Collatz conjecture, also known as the _3n + 1 problem_ is unsolved problem, which states, that:

If a given function, described bellow, is applied to every positive integer, the value will eventually
result to 1.

If the given value is even, then it is to be devided by 2. Otherwise it is to be multiplied by 3 and 1 added.

In modular arithmetic notation, this function _f_ can be defined as:
"""

@equation
fun f(n) {
    if(n % 2 == 0) return n/2
    else return 3*n + 1
}

md"""
In Moss, a method to calculate resulting value for a given integer, can be written as bellow.

Intermediete values and total amount of steps is being counted to better showcase how this function works.
"""

fun collatz(n) {
    d"""
    Collatz conjecture test
    @param n Value to test the collatz conjecture for
    @return nil
    @note Prints intermediate values to the stdout
    """
    while(n != 1) {
        if(n % 2 == 0) {
            n /= 2
        }
        else {
            n = 3*n + 1
        }
        n++" "
    }
}

md"""
Invoking this function for a random value, gives the following output
"""
~collatz(42)
"\n\n"

md"""
We can now look at how the number of steps differs for different values using the following function:
"""
fun collatz_steps(n) {
    d"""
    Collatz conjecture test that returns the number of steps needed
    @param n Value to test the collatz conjecture for
    @return Number of steps needed for the value to get to 1
    """
    steps = 0
    while(n != 1) {
        if(n % 2 == 0) {
            n /= 2
        }
        else {
            n = 3*n + 1
        }
        steps += 1
    }
    return steps
}

md"""
Lets first calculate the number of steps for _n < 16_: 
"""
tbl = [0]
for(i : 1..16) {
    tbl += [collatz_steps(i)]
}

"Number of steps for n <= 15:\n"
@table
tbl
"\n\n"

md"""
Lets now plot the number of steps for even more values to a graph:
"""
tbl = [0]
for(i : 1..501) {
    tbl += [collatz_steps(i)]
}

"Number of steps for n <= 500:\n"
tbl
#~Grph::plot(tbl)
