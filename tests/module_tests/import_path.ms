import "mod1.ms"
import "../module_tests/mod2.ms"

mod2.mod2fun(4)
"\n"

import sys
if (sys.platform() == sys.Platform.Linux) {
    ~with_open("/tmp/created_module.ms", fun (f) = f.write("42"), "w")
    import "/tmp/created_module.ms"
    import "/tmp/"/created_module
    import "/tmp/created_module.ms" as mcm
    assert(created_module == mcm, "not equal same module")
} else {
    42
}
"\n"

import "../module_tests/"/mod2.Sum as mod2sum
mod2sum(4).sum(1, 2)
"\n"
