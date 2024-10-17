fun fib(n) {
    //d"""Fibonacci sequence
    //@param n Index for desired number in the sequence
    //@return Value in Fibonacci sequnce for passed in index
    //"""
    if(n < 2) return n
    else return fib(n-1) + fib(n-2)
}

fib(2)
"\n"
fib(10)
"\n"
fib(13)
"\n"
fib(18)
"\n"
