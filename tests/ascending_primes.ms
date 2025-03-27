fun is_prime(n) {
    if (n == 2) return true
    if (n == 1 or n % 2 == 0) return false
    
    root = int(n^0.5 + 1)
    
    for (k : 3, 5..root) {
        if (n % k == 0) return false
    }

    return true
}

queue = [k : k = (1..10)]
primes = []

while(!queue.empty()) {
    n = queue.pop()
    if (is_prime(n))
        ~primes.append(n)
    // ????
    ~queue.extend([n * 10 + k : k = (n % 10 + 1 .. 10)])
}

primes