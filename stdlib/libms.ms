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

