{:}
"\n"
{10: "0xA", 3: "3", 10 : "10"}
"\n"
{true: "true", false: "false", nil: "nil", 0: "0", "0_true": "0_true"}
"\n"
d1 = {1: 1, 2: 2, 2.0: 2.0}
d1
"\n"

try {
    {1: 2, [1,2]: 3}
} catch (e:TypeError)
    "caught\n"

try {
    {1: 2, {:}: 3}
} catch (e:TypeError)
    "caught\n"

// Dictionary with integer keys and string values
d = {1: "one", 2: "two", 3: "three"}
d++"\n"

// Dictionary with string keys and integer values
d = {"a": 1, "b": 2, "c": 3}
d++"\n"

// Dictionary with mixed keys and string values
d = {nil: "n1", false: "f2"}
d++"\n"
d = {1: "int", "key": 2.5, Int: [1, 2]}
d++"\n"

// Dictionary with list values
d = {1: [1, 2, 3], 2: [4, 5, 6]}
d++"\n"

// Dictionary with boolean keys and values
d = {true: "yes", false: "no"}
d++"\n"

// Dictionary with float keys and string values
d = {3.14: "pi", 2.718: "e"}
d++"\n"

// Dictionary with nil as a key and integer value
d = {nil: "nothing", "key": 42}
d++"\n"

// Dictionary with nested dictionaries (dictionaries as values)
d = {"outer": {"inner": "value"}}
d++"\n"

// Dictionary with empty values (empty strings, lists, dictionaries)
d = {"empty_str": "", "empty_list": [], "empty_dict": {:}}
d++"\n"

// Dictionary with custom objects as keys (as long as they are hashable)
class CustomObject {
    fun CustomObject(val) {
        this.val = val
    }

    fun __hash() = this.val
    fun (==)(other) = this.val == other.val
    fun __String() = "CustomObject: "++this.val
}

obj1 = CustomObject(1)
obj2 = CustomObject(2)
obj3 = CustomObject(1)

d = {obj1: "custom1", obj2: "custom2"}
d++"\n"

d = {obj1: "custom1", obj2: "custom2", obj3: "custom1_obj3"}
d++"\n"

class C{}
try {
    {C(): "C", C(): "B"}
} catch (e:TypeError)
    "caught\n"

class CstCls {
    fun CstCls(x) {
        this.x = x
    }
    fun __String() {
        return "CustomClass: " ++ this.x
    
    }
    fun __hash() = this.x
    fun (==)(other) = this.x == other.x
}

dct = {"a": 3, 3: "a", Int: "int", CstCls(3): "cst_cls3"}

try
    dct["3"]
catch (e:KeyError)
    "caught\n"

try
    dct[CstCls(6)]
catch(e: KeyError)
    "caught\n"

dct["a"]
"\n"
dct[3]
"\n"
dct[Int]
"\n"
dct[CstCls(3)]
"\n"

a = {:}
a["a"] = 42
a
"\n"
a["a"] = 68
a
"\n"

a[true] = true
a[1] = true
a
"\n"

first = true
for (kv: a) {
    if (not first)
        ", "
    kv[0] ++ ": " ++ kv[1]
    first = false
}
"\n"

for (kv: {:}) {
    kv
}

for (kv: {"a": 1, "b": 2}) {
    kv ++ "\n"
}