class MyString : String {
    fun foo() {
        return this.__value
    }

    fun ([])(index) {
        return MyString(this.__value[index])
    }

    fun __String() {
        return "[MS]"++this.__value
    }

    fun __iter() {
        return super.__iter()
    }
    
}

class MyString2 : String {

    fun __iter() {
        this.i = super.__iter()
        return this
    }

    fun __next() {
        return this.i.__next()
    }
    
}

ms = MyString("hello")
type(ms)
"\n"
ms.length()
"\n"
ms.foo()
"\n"
ms.__value = 55 
try {
    ms.length()
} catch (e:TypeError)
    "caught\n"

~delattr(ms, "__value")
try {
    ms.length()
} catch (e:AttributeError)
    "caught\n"

ms = MyString("abcdefg")
ms.empty()
"\n"

MyString("  a\n  ").strip()
"\n"
try
    String.strip(52)
catch (e:ValueError)
    "caught\n"

ms.reverse().__value
"\n"

ms.capitalize()
"\n"

ms2 = MyString(ms.upper())
ms2.foo()
"\n"
ms2.lower()
"\n"

ms.replace("a", "-")
"\n"
ms.multi_replace([["a", "-"], ["b", "-"], ["c", "-"]])
"\n"

MyString(",").join([1,2,3])
"\n"

for (l: ms) {
    l
}
"\n"

for (l: MyString2("123"))
    l
"\n"
