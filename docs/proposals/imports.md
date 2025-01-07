# How to include stdlib into programs

* We need moss code inclusion for easy change of lib functions and classes
* This code should be compilable for faster import
* It should pretty much work as `import libms::*`

# General imports into name

When importing not into global scope but into name
```py
import Rng
```

We can simply parse the `Rng.ms` or `Rng.msb` into bytecode, push new frame
and interpret the code, this frame can be then stored into `ModuleValue` value.

# Imports into global scope

When importing into global scope
```py
import Rng::*
```

We do the same as before, but this time we push this ModuleValue not under it's
name, but into a list of spilled modules.

When we then search for a name, we:
1. Go through local frame
2. Go through list of spilled modules in this frame from the back
3. Go through global frame
4. Go through list of spilled modules in global frame from the back

# Partial imports

```py
import MathAdv::PI
```

Once again the whole module has to be imported as in other cases (so also
interpreted), but this time only the imported value will be saved in the global
scope under the desired name.