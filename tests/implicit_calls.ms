/// Test for calling runtime implicit function such
/// as __String when outputting an object

class MyString {

    fun MyString(txt) {
        this.txt = txt
    }

    fun __String() {
        return this.txt ++ " is my string";
    }
}

mystr = MyString("hello")
mystr
"\n"

mystr2 = MyString("bye")
mystr2 ++ "\n"
