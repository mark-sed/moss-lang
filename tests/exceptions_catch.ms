try {
    a
} catch (e:NameError) {
    "NameError: a\n"
}

try {
    foo()
} catch (e:NameError) {
    "NameError: foo()\n"
}