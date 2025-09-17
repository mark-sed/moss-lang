try {
    fun foo() {
        @!unknown_annotation(3)
    }
} catch (x:NameError)
    "caught\n"

try
    @!unknown_module_annotation(35)
catch (e:NameError)
    "caught\n"

try {
    @unknown_class_annotation
    class V {}
} catch (e:NameError)
    "caught\n"