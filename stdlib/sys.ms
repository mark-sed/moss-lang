d"""
This module provides resources tied to current running Moss interpreter and
the system this interpreter is running on.
"""

version="" //< This value will be set dynamically

space version_info {
    major=0
    minor=0
    patch=0
    build_type=""

    // TODO: Add function for checking if version is at least or more than...
}

enum Platform {
    Windows,
    Linux,
    Darwin,
    Unknown,
}

@internal
fun platform() {}

@internal
fun getenv(name:String) {}
@internal
fun getenv(name:String, def_val) {}

// This annotation has to be last so that
// it can access values defined above.
@!internal_module