import sys

version = sys.version
for (i: 0..version.length()) {
    if (version[i] == " ") {
        version = version[0..i]
        break
    }
}
conc_ver = sys.version_info.major ++ "." ++ sys.version_info.minor ++ "." ++ sys.version_info.patch

assert(version == conc_ver, "Version mismatch?")

bt = sys.version_info.build_type
assert(bt == "debug" || bt == "release", "Other build type")
