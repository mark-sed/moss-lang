/**
 * Fibonacci sequence
 * @param n Index for desired number in the sequence
 * @return Value in Fibonacci sequnce for passed in index
 */
fun fib(n) {
    if(n < 2) return n
    else return fib(n-1) + fib(n-2)
}

fib(10)
