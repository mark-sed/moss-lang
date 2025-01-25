///
/// \brief Moss standard library
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///

__LIB_MS_VERSION = "0.0.1"

@internal_bind("Int")
class __Int {
    @internal
    fun Int(v:String, base=10) {}

    @internal
    fun Int(v:[Int, Float]=0) {}
}

/*@internal_bind("Float")
class __Float {
    @internal
    fun Float(v:String) {}

    @internal
    fun Float(v:[Int, Float]=0.0) {}
}*/

@internal
fun vardump(value) {}

@internal
fun print(... msgs, end="\n", separator=" ") {}

@internal
fun exit(code=0) {}

/// Exceptions
// TODO: Place into space
@internal_bind("Exception")
class __Exception {
    @internal
    fun Exception(msg:String) {}
}
