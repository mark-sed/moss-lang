d"""
This module provides resources tied to current running Moss interpreter and
the system this interpreter is running on.
"""

version="" //< Moss version (this value will be set dynamically).

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

@internal
fun get_path() {
    d"""
    Returns the currently used lookup path for modules.
    """
}

@internal
fun set_path(... paths) {
    d"""
    Sets the lookup path for modules to passed in values.
    """
}

@internal
fun add_path(path:String, prepend:Bool=true) {
    d"""
    Appends `path` to current lookup path for modules.
    If `prepend` is true then this value is prepended at the front of the full
    path, when false then at the end.
    """
}

// This annotation has to be last so that
// it can access values defined above.
@!internal_module