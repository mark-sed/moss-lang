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

// Uncomment once added
/*fun foo(a:Rnd, b) { 
    return a::shuffle(b) 
}; */

fun foo(a, b) {
    return "All other types\n"
}

foo(1, 2)
foo("1", 2)
//foo(Rnd, [2,3,4,5])
foo(true, false)


/**
 * Type inheritance
 * A function accept all child classes as they are subtypes of their
 * parent type
 */

class Acls {
    fun Acls(id) {
        this.id = id
    }
}

class Bcls : Acls {

}

fun get_unique_id(a:Acls) {
    return a.id
}

acls1 = Acls(1)
bcls2 = Bcls(2)

get_unique_id(acls1)
"\n"
get_unique_id(bcls2)
"\n"

