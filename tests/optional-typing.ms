/**
 * This program showcases optional argument typing for functions
 * and function overloading
 *
 * When function foo is called the interpreter checks not only for the 
 * argument amount, but also if the types match (if used) and picks the
 * most suitable candidate as the function that will be called.
 *
 * If function call does not match any function definition by its types 
 * (and there is no function that would accept any type), 
 * then a TypeError is raised.
 */

fun foo(a:Int, b) { 
    return "Int first\n"
}; 

fun foo(a:String, b) { 
    return "String first\n"
}; 

fun foo(a:Rnd, b) { 
    return a::shuffle(b) 
}; 

fun foo(a, b) {
    return "\nAll other types\n"
}

foo(1, 2)
foo("1", 2)
foo(Rnd, [2,3,4,5])
foo(true, false)

/** Output: */
# first
# redef
# [5, 4, 2, 3]
# all else

