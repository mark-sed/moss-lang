fun collatz(n) {
    //d"""
    //Collatz conjecture test
    //@param n Value to test the collatz conjecture for
    //@return nil
    //@note Prints intermediate values to the stdout
    //"""
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

~collatz(42)
"\n"
~collatz(4194304)
"\n"
~collatz(7)