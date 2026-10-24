// Compile with `gcc -fPIC -shared -o clib.so clib.c`
// On Windows with `cl /LD clib.c /Fe:clib.dll`
#include <stdio.h>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

EXPORT void greet() {
    fprintf(stderr, "Hello from C!\n");
}

EXPORT long get_num() {
    return 42L;
}

EXPORT double add(double a, double b) {
    return a + b;
}

EXPORT double add_dl(double a, long b, long c, long d) {
    return a + b + c + d;// + e + f + g + h + i + j;
}

EXPORT double add_dl10(double a, long b, long c, long d, long e, long f, long g, long h, long i, long j) {
    return a + b + c + d + e + f + g + h + i + j;
}

EXPORT void print_msg(const char *msg) {
    fprintf(stderr, msg);
}

const char *LIB_NAME = "CLIB";

EXPORT const char *get_lib_name() {
    return LIB_NAME;
}