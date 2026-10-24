use std::ffi::{c_char, CStr};
use std::os::raw::c_ulonglong;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn print_and_length(s: *const c_char) -> u64 {
    if s.is_null() {
        return 0;
    }

    let cstr = unsafe { CStr::from_ptr(s) };
    eprintln!("{}", cstr.to_string_lossy());

    cstr.to_bytes().len() as u64
}

#[unsafe(no_mangle)]
pub extern "C" fn add(a: i32, b: i32) -> i32 {
    a + b
}