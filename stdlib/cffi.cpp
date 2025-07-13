#include "cffi.hpp"
#include "values_cpp.hpp"
#include <unordered_map>

#ifndef __windows__
#include <dlfcn.h>
#else
#include <windows.h>
#endif

using namespace moss;
using namespace mslib;
using namespace cffi;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& cffi::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"dlopen", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return cffi::dlopen(vm, cf, args[0].value, err);
        }},
    };
    return registry;
}

#ifdef __windows__
static Value *windows_dlopen(Interpreter *vm, ustring path, Value *&err) {
    assert(false && "TODO")
    return nullptr;
}
#else
static Value *posix_dlopen(Interpreter *vm, ustring path, Value *&err) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        const char *dlsym_error = dlerror();
        err = create_not_implemented_error(ustring("cffi.dlopen failed, but exception is not implemented. ")+dlsym_error);
        return nullptr;
    }
    return new t_cpp::VoidStar(handle);
}
#endif

Value *cffi::dlopen(Interpreter *vm, CallFrame *cf, Value *path, Value *&err) {
    auto path_str = dyn_cast<StringValue>(path);
    assert(path_str && "non-string");
    Value *handle = nullptr;
#ifdef __windows__
    err = create_not_implemented_error("cffi.dlopen is not yet implemented for Windows systems.\n");
    return nullptr;
#else
    handle = posix_dlopen(vm, path_str->get_value(), err);
#endif
    return mslib::call_constructor(vm, cf, "CFFI", {handle}, err);
}