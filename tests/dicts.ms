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

// TODO: Uncomment once added
// Dictionary with custom objects as keys (as long as they are hashable)
//class CustomObject {
//    fun CustomObject(val) {
//        this.val = val
//    }
//
//    fun __hash() = hash(this.val)
//    fun (==)(other) = this.val == other.val
//}
//
//obj1 = CustomObject(1)
//obj2 = CustomObject(2)
//
//d = {obj1: "custom1", obj2: "custom2"}
//d++"\n"