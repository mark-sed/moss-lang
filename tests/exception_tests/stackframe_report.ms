fun foo() {
    space FOOS {
        class FooCls {
            @static_method
            fun inner() {
                raise 4
            }

            return FooCls.inner()
        }
    }
}

try
    ~foo()
catch (e) {
    "caught\n"
    foo()
}