fun is_prime(n) {
    if (n == 2) return true
    if (n == 1 or n % 2 == 0) return false
    
    root = Int(n^0.5) + 1
    
    for (k : 3, 5..root) {
        if (n % k == 0) return false
    }

    return true
}

queue = [k : k = (1..10)]
primes = []

while(queue.length() != 0) {
    n = queue.pop(0)
    if (is_prime(n))
        ~primes.append(n)
    queue += [n * 10 + k : k = (n % 10 + 1 .. 10)]
}

primes
