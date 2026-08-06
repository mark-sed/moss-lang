import python

test = python.module("py_calls_python")
vardump(test.foo.call)
vardump(test.foo.call_moss)
vardump(test.bar.call)
r = test.foo(1, 2, 3, 4, 5, 6, 7, e=9)
r.to_moss()
"\n"
r2 = test.foo.call(1, 2, 3, 4, 5, 6, 7, e=9, f=-2)
r2.to_moss()
"\n"
test.foo.call_moss(1, 2, 3, 4, 5, 6, 7, e=9, f=-2)
"\n"

// Basic positional arguments
test.add(2, 3).to_moss()                     // 5
"\n"
test.add.call_moss(a=2, b=3)                 // 5
"\n"
test.add(2, b=3).to_moss()                   // 5
"\n"

// Default arguments
test.greet("Alice").to_moss()                // "Hello, Alice!"
test.greet.call_moss("Alice", "Hi")          // "Hi, Alice!"
test.greet.call(name="Bob").to_moss()             // "Hello, Bob!"
test.greet(greeting="Hey", name="Bob").to_moss()  // "Hey, Bob!"

// Positional-only
test.pos_only.call_moss(4, 5)                // 20
"\n"

// Keyword-only
test.kw_only(x=5).to_moss()                  // 15
"\n"
test.kw_only(x=5, y=7).to_moss()             // 12
"\n"

// *args
test.collect().to_moss()                     // ()
"\n"
test.collect(1).to_moss()                    // (1,)
"\n"
test.collect.call_moss(1, 2, 3)              // (1, 2, 3)
"\n"

// **kwargs
test.collect_kwargs().to_moss()              // {}
"\n"
test.collect_kwargs({"a":1}).to_moss()           // {"a": 1}
"\n"
test.collect_kwargs({"a": 1, "b": 2}).to_moss()      // {"a": 1, "b": 2}
"\n"

// Mixed signature
test.mixed(1, c=3).to_moss()
"\n"
// {"a":1, "b":2, "args":(), "c":3, "d":4, "kwargs":{}}

test.mixed(1, 5, 6, 7, c=8).to_moss()
"\n"
// {"a":1, "b":5, "args":(6,7), "c":8, "d":4, "kwargs":{}}

test.mixed(1, c=3, kwargs={"hello": 42}).to_moss()
"\n"
// {"a":1, "b":2, "args":(), "c":3, "d":4, "kwargs":{"hello":42}}

test.mixed(1, 5, 6, c=7, d=8, kwargs={"x": 9, "y":10}).to_moss()
"\n"
// {"a":1, "b":5, "args":(6,), "c":7, "d":8, "kwargs":{"x":9,"y":10}}

// Everything
test.everything(1, d=10).to_moss()
"\n"
// {"a":1, "b":2, "c":3, "args":(), "d":10, "e":5, "kwargs":{}}

test.everything(1, 20, 30, 40, 50, d=60).to_moss()
"\n"
// {"a":1, "b":20, "c":30, "args":(40,50), "d":60, "e":5, "kwargs":{}}

test.everything(1, 20, d=30, e=40, kwargs={"foo": 50}).to_moss()
"\n"
// {"a":1, "b":20, "c":3, "args":(), "d":30, "e":40, "kwargs":{"foo":50}}

// Argument unpacking
arg = [10, 20]
test.add.call_moss(<<arg)                    // 30
"\n"

arg = ["Charlie"]
test.greet(<<arg).to_moss()                  // "Hello, Charlie!"

kwargs = {"name": "Dana", "greeting": "Hi"}
test.greet(<<kwargs).to_moss()               // "Hi, Dana!"

arg = [1, 2, 3, 4]
kwarg = {"x": 7}
test.everything(<<arg, d=5, e=6, kwargs=kwarg).to_moss()
"\n"
// {"a":1, "b":2, "c":3, "args":(4,), "d":5, "e":6, "kwargs":{"x":7}}

