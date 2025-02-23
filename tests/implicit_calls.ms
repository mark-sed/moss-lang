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

class MyInt {
    fun __String() {
        return 42
    }
}

// Int will be returned but converted to string
MyInt()++"\n"

class MyNumber {
    fun __String() {
        return MyNumber()
    }
}

// MyString will be returned but as string wihout calling __String
// this is to avoid recursive issues
MyNumber()++"\n"

class MyList {
    fun MyList(lst) {
        this.lst = lst
    }

    fun __String(a) {
        return "bbb - "++this.lst
    }

    fun __String() {
        return "AAA - "++this.lst
    }

    fun __String(c) {
        return "cc - "++this.lst
    }
}

lst = MyList([1,2,3])
lst
"\n"