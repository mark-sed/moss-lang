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

class MyClass {

    new MyClass(self, v) {
        self.v = v
    }

    fun __String(self) {
        return "MyClass("++String(self.v)++")"
    }

    fun __Int(self) {
        return self.v
    }

    fun __Float(self) {
        return Float(self.v)
    }

    fun __Bool(self) {
        return self.v != 0
    }
}

a = MyClass(42)

# Cast has to be explicitly specified, methods won't be called otherwise
String(a)++"\n"
Int(a)++"\n"
Float(a)++"\n"
Bool(a)++"\n"