d = {:}

// Insert elements
d["a"] = 1
d["b"] = 2
d["c"] = 3

~print(d)  // {"a": 1, "b": 2, "c": 3}

// Overwrite existing key (should NOT change order)
d["b"] = 20
~print(d)  // {"a": 1, "b": 20, "c": 3}

// Insert new key
d["d"] = 4
~print(d)  // {"a": 1, "b": 20, "c": 3, "d": 4}

// Remove middle element
d.pop("b")
"\n"
~print(d)  // {"a": 1, "c": 3, "d": 4}

// Reinsert same key (should go to end)
d["b"] = 200
~print(d)  // {"a": 1, "c": 3, "d": 4, "b": 200}

// Remove using remove (if different from pop)
~d.remove("c")
~print(d)  // {"a": 1, "d": 4, "b": 200}

// pop_item (assume removes first inserted)
d.pop_item()
"\n"
~print(d)  // {"d": 4, "b": 200}

// Add more again
d["x"] = 100
d["y"] = 200
~print(d)  // {"d": 4, "b": 200, "x": 100, "y": 200}

// Clear everything
~d.clear()
~print(d)  // {:}

//---- some nesting
d = {:}

// Insert simple values
d["x"] = 1
d["y"] = 2

// Insert nested dict
d["nested"] = {:}
d["nested"]["a"] = 10
d["nested"]["b"] = 20

~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 10, "b": 20}}""")

// Insert list with dicts
d["list"] = []
~d["list"].append({"k1": 100})
~d["list"].append({"k2": 200})

~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 10, "b": 20}, "list": [{"k1": 100}, {"k2": 200}]}""")

// Modify nested dict (overwrite should not reorder)
d["nested"]["a"] = 999
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "b": 20}, "list": [{"k1": 100}, {"k2": 200}]}""")

// Add new key to nested dict (goes to end inside nested)
d["nested"]["c"] = 30
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "b": 20, "c": 30}, "list": [{"k1": 100}, {"k2": 200}]}""")

// Remove from nested dict
d["nested"].pop("b")
"\n"
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "c": 30}, "list": [{"k1": 100}, {"k2": 200}]}""")

// Reinsert into nested (should go to end)
d["nested"]["b"] = 222
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "c": 30, "b": 222}, "list": [{"k1": 100}, {"k2": 200}]}""")

// pop_item removes LAST inserted (top-level)
d.pop_item()
"\n"
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "c": 30, "b": 222}}""")

// Add new complex element
d["z"] = {"inner": [ {"p": 1}, {"q": 2} ]}
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {"a": 999, "c": 30, "b": 222}, "z": {"inner": [{"p": 1}, {"q": 2}]}}""")

// Clear nested dict only
~d["nested"].clear()
~print(d)
assert(String(d) == """{"x": 1, "y": 2, "nested": {:}, "z": {"inner": [{"p": 1}, {"q": 2}]}}""")

d.pop_item()
"\n"
d.pop_item()
"\n"
d.pop_item()
"\n"
d.pop_item()
"\n"