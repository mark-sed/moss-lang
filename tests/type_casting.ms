/**
 * Programs showcasing how to implement custom
 * methods to cast an object to built-in type.
 * Built-in type constructors call `__X(self)` methods
 * on the given object, where the X stands for the type
 * name.
 *
 * Available are:
 *   __String
 *   __Int
 *   __Float
 *   __Bool
 */

class MyValue {
    fun MyValue(x) {
        this.x = x
    }

    fun __Int() {
        return this.x
    }

    fun __String() {
        // This will be casted to string once 
        // returned unlike other ones, which will
        // raise
        return this.x
    }

    fun __Float() {
        return Float(this.x)
    }

    fun __Bool() {
        return this.x > 0
    }
}

class TheirValue {}

tv = TheirValue()
mv = MyValue(42)

// Cast has to be explicitly specified, methods won't be called
Int(mv)++"\n"
Bool(mv)++"\n"
String(mv)++"\n"
Float(mv)++"\n"

try
    Int(tv)
catch(e:TypeError)
    "caught\n"

try
    Float(moss)
catch(e:TypeError)
    "caught\n"

class IncorrectRet {
    fun __Int() {
        return true
    }
}

try
    Int(IncorrectRet())
catch (e:TypeError)
    "caught\n"