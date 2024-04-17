/**
 * This programs prints the first 15 anti-primes
 */

fun cntDivs(n) {
    if (n < 2) return 1
    cnt = 2
    for(i : 2..n/2) {
        if(n % i == 0) cnt += 1
    }
    return cnt
}

max_div = 0
cnt = 0
n = 1
"The first 15 anti-primes are:\n"
while(cnt < 15) {
    d = cntDivs(n)
    if(d > max_div) {
        n++"\n"
        max_div = d
        cnt+=1
    }
    n+=1
}
