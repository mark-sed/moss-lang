// Compile with `gcc -fPIC -shared -o clib.so clib.c`
#include <stdio.h>

void greet() {
    puts("Hello from C!\n");
}

long get_num() {
    return 42L;
}

double add(double a, double b) {
    return a + b;
}

long add_dl() {
    return 8L;
}