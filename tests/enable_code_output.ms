md"""# Using code output
When enable_code_output is set then all the code
will be outputted as well.
"""

@converter("txt", "md")
fun txt2md(x) = x

@!enable_code_output

fun foo() {
    /// @returns string
    return "hi\n"
}

foo()

@!disable_code_output

fun foo2() {
    return 2
}

md"""## Here the code output was disabled
We get some value but it is not guarded as [Output]
"""

foo2()
"\n"

if (true) {

} else {

}

assert(true, "hi")