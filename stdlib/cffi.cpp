#include "cffi.hpp"
#include "values_cpp.hpp"
#include <unordered_map>
#include <ffi.h>
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
        {"define", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 4);
            return cffi::define(vm, cf, cf->get_arg("this"), cf->get_arg("name"), cf->get_arg("return_type"), cf->get_arg("arg_types"), err);
        }},
        {"close", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return cffi::dlclose(vm, args[0].value, err);
        }},
    };
    return registry;
}

static t_cpp::CVoidStarValue *get_handle(Value *obj, Interpreter *vm, Value *&err) {
    auto handle = mslib::get_attr(obj, "handle", vm, err);
    if (!handle)
        return nullptr;
    auto handle_val = dyn_cast<t_cpp::CVoidStarValue>(handle);
    // TODO: Perhaps change this to some error with the note that the type has changed
    assert(handle_val && "Attribute handle was changed");
    return handle_val;
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
        err = create_not_implemented_error("cffi.dlopen failed, but exception is not implemented. "+ustring(dlsym_error)+"\n");
        return nullptr;
    }
    return new t_cpp::CVoidStarValue(handle);
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
    if (!handle)
        return nullptr;
    return mslib::call_constructor(vm, cf, "CFFI", {handle}, err);
}

Value *cffi::dlclose(Interpreter *vm, Value *ths, Value *&err) {
#ifdef __windows__
    err = create_not_implemented_error("cffi.dlclose is not yet implemented for Windows systems.\n");
    return nullptr;
#else
    auto handle = get_handle(ths, vm, err);
    if (!handle)
        return nullptr;
    ::dlclose(handle->get_value());
#endif
    return nullptr;
}

Value *cffi::define(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *return_type, Value *arg_types, Value *&err) {
    auto name_s = mslib::get_string(name);
    auto argst = mslib::get_list(arg_types);

    auto handle = get_handle(ths, vm, err);
    if (!handle)
        return nullptr;

    void *func = dlsym(handle->get_value(), name_s.c_str());
    
    ffi_cif cif;
    std::vector<ffi_type *> args;
    for (auto a: argst) {
        assert(false && "TODO");
    }
    // TODO: Convert
    ffi_type *ffi_ret_type = &ffi_type_void;

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), ffi_ret_type, args.data())) {
        err = create_not_implemented_error("cffi.define failed, but exception is not implemented.\n");
        return nullptr;
    }
    
    auto func_v = new t_cpp::CVoidStarValue(func);
    auto cif_v = new t_cpp::Ffi_cifValue(cif);

    auto ffhandle = mslib::call_constructor(vm, cf, "FFHandle", {func_v, cif_v}, err);
    if (!ffhandle)
        return nullptr;
    
    ths->set_attr(name_s, ffhandle);
    return nullptr;
}