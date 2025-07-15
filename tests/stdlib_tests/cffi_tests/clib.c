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

double add_dl(double a, long b, long c, long d) {
    return a + b + c + d;// + e + f + g + h + i + j;
}

double add_dl10(double a, long b, long c, long d, long e, long f, long g, long h, long i, long j) {
    return a + b + c + d + e + f + g + h + i + j;
}

void print_msg(const char *msg) {
    puts(msg);
}

const char *LIB_NAME = "CLIB";

const char *get_lib_name() {
    return LIB_NAME;
}