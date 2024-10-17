fun fact(n) {
    if (n <= 0) return 1
    else return n * fact(n-1)
}

// Biggest n, where n! fits into long
fact(20)