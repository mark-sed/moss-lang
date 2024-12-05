/**
 * Programs showcasing how to implement custom
 * methods to cast an object to built-in type.
 * Built-in type constructors call `__X(this)` methods
 * on the given object, where the X stands for the type
 * name.
 *
 * Available are:
 *   __String
 *   __Int
 *   __Float
 *   __Bool
 */

class MyClass {

    fun MyClass(v) {
        this.v = v
    }

    fun __String() {
        return "MyClass("++String(this.v)++")"
    }

    fun __Int() {
        return this.v
    }

    fun __Float() {
        return Float(this.v)
    }

    fun __Bool() {
        return this.v != 0
    }
}

a = MyClass(42)

// Cast has to be explicitly specified, methods won't be called otherwise
String(a)++"\n"
Int(a)++"\n"
Float(a)++"\n"
Bool(a)++"\n"