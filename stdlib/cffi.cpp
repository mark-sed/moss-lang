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

union FFIResult {
    int cint;
    unsigned int cunsigned_int;
    short cshort;
    unsigned short cunsigned_short;
    char cchar;
    unsigned char cunsigned_char;
    long clong;
    unsigned long cunsigned_long;
    float cfloat;
    double cdouble;
    void *cvoid_star;
};

const std::unordered_map<std::string, mslib::mslib_dispatcher>& cffi::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"call", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            return cffi::call(vm, cf, cf->get_arg("this"), cf->get_arg("args"), err);
        }},
        {"cfun", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 4);
            return cffi::cfun(vm, cf, cf->get_arg("this"), cf->get_arg("name"), cf->get_arg("return_type"), cf->get_arg("arg_types"), err);
        }},
        {"close", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return cffi::dlclose(vm, args[0].value, err);
        }},
        {"dlopen", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return cffi::dlopen(vm, cf, args[0].value, err);
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
    HMODULE handle = LoadLibrary(path.c_str());
    if (!handle) {
        err = create_not_implemented_error("cffi.dlopen failed, but exception is not implemented.\n");
        return nullptr;
    }
    return new t_cpp::CVoidStarValue(handle);
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
    handle = windows_dlopen(vm, path_str->get_value(), err);
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

static ffi_type* get_ffi_type(Value *value, Interpreter *vm, Value *&err) {
    bool is_class = isa<ClassValue>(value);
    Value *type = is_class ? value : value->get_type();
    if (!is_class && !dyn_cast<t_cpp::CppValue>(value)) {
        err = mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_CPP_MOSS_VALUE, type->get_name().c_str()));
        return nullptr;
    }
    static const std::unordered_map<std::string, ffi_type*> type_map = {
        {"cvoid",   &ffi_type_void},
        {"cint",    &ffi_type_sint},
        {"cunsigned_int", &ffi_type_uint},
        {"cshort",  &ffi_type_sshort},
        {"cunsigned_short", &ffi_type_ushort},
        {"cchar",   &ffi_type_schar},
        {"cunsigned_char", &ffi_type_uchar},
        {"clong",   &ffi_type_slong},
        {"cunsigned_long", &ffi_type_ulong},
        {"cfloat",  &ffi_type_float},
        {"cdouble", &ffi_type_double},
        {"cvoid_star", &ffi_type_pointer},
        {"cchar_star", &ffi_type_pointer}, // TODO: When the type has _star then return always this
    };

    auto it = type_map.find(type->get_name());
    if (it != type_map.end()) {
        return it->second;
    } else {
        if (is_class) {
            err = mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_CPP_MOSS_VALUE, type->get_name().c_str()));
        } else {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_KNOWN_TYPE_CONV_TO_C, type->get_name().c_str()));
        }
        return nullptr;
    }
}

static Value *result_to_moss(FFIResult result, Value *type, Value *&err) {
    assert(type != BuiltIns::Cpp::CVoid && "invoked with void");
    /*if (type == &ffi_type_sint)
        return new IntValue(result.cint);
    if (type == &ffi_type_uint)
        return new IntValue(result.cunsigned_int);
    if (type == &ffi_type_sshort)
        return new IntValue(result.cshort);
    if (type == &ffi_type_ushort)
        return new IntValue(result.cunsigned_short);
    if (type == &ffi_type_schar)
        return new IntValue(result.cchar);
    if (type == &ffi_type_uchar)
        return new IntValue(result.cunsigned_char);*/
    if (type == BuiltIns::Cpp::CLong)
        return new IntValue(result.clong);
    /*if (type == &ffi_type_ulong)
        return new IntValue(result.cunsigned_long);
    if (type == &ffi_type_float)
        return new FloatValue(result.cfloat);*/
    if (type == BuiltIns::Cpp::CDouble)
        return new FloatValue(result.cdouble);
    if (type == BuiltIns::Cpp::CCharStar)
        return new StringValue(ustring(static_cast<char *>(result.cvoid_star)));
    if (type == BuiltIns::Cpp::CVoidStar)
        return new t_cpp::CVoidStarValue(result.cvoid_star);

    err = mslib::create_not_implemented_error("Conversion for returned type is not yet implemented in cffi\n");
    return nullptr;
}

Value *cffi::cfun(Interpreter *vm, CallFrame *cf, Value *ths, Value *name, Value *return_type, Value *arg_types, Value *&err) {
    auto name_s = mslib::get_string(name);
    auto argst = mslib::get_list(arg_types);

    auto handle = get_handle(ths, vm, err);
    if (!handle)
        return nullptr;

#ifdef __windows__
    void *func = nullptr;
    err = create_not_implemented_error("cffi.define is not yet implemented on Windows.\n");
    return nullptr;
#else
    void *func = dlsym(handle->get_value(), name_s.c_str());
#endif

    ffi_cif cif;
    std::vector<ffi_type *> *args = new std::vector<ffi_type *>();
    // We need to store args to then be able to delete them.
    // TODO: Create special value just for this to then not delete void *, but the actual type.
    auto args_ptr = new t_cpp::CVoidStarValue(args, true);
    for (auto a: argst) {
        auto convv = get_ffi_type(a, vm, err);
        if (!convv)
            return nullptr;
        args->push_back(convv);
    }
    ffi_type *ffi_ret_type = get_ffi_type(return_type, vm, err);
    if (!ffi_ret_type)
        return nullptr;

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args->size(), ffi_ret_type, args->data())) {
        err = create_not_implemented_error("cffi.define failed, but exception is not implemented.\n");
        return nullptr;
    }
    
    auto func_v = new t_cpp::CVoidStarValue(func);
    auto cif_v = new t_cpp::Ffi_cifValue(cif);

    auto ffhandle = mslib::call_constructor(vm, cf, "FFHandle", {func_v, cif_v, name, return_type, arg_types, args_ptr}, err);
    if (!ffhandle)
        return nullptr;
    
    ths->set_attr(name_s, ffhandle);
    return nullptr;
}

Value *cffi::call(Interpreter *vm, CallFrame *cf, Value *ths, Value *args, Value *&err) {
    auto ptr = mslib::get_attr(ths, "ptr", vm, err);
    if (!ptr)
        return nullptr;
    auto ptrv = dyn_cast<t_cpp::CVoidStarValue>(ptr);
    assert(ptrv && "not void*");

    auto cif = mslib::get_attr(ths, "cif", vm, err);
    if (!cif)
        return nullptr;
    auto cifv = dyn_cast<t_cpp::Ffi_cifValue>(cif);
    assert(cifv && "not cif");

    auto argsv = mslib::get_list(args);
    // TODO: Typecheck arguments
    std::vector<void *> values;
    for (auto a: argsv) {
        values.push_back(a->get_data_pointer());
    }
    auto return_type = mslib::get_attr(ths, "return_type", vm, err);
    if (!return_type)
        return nullptr;

    auto ffi_ret_t = get_ffi_type(return_type, vm, err);
    if (!ffi_ret_t)
        return nullptr;
    bool void_fun = ffi_ret_t == &ffi_type_void;
    FFIResult result;

    // TODO: Maybe store cif in Ffi_cifValue as a pointer to not copy as much
    auto cif_val = cifv->get_value();
    ffi_call(&cif_val, FFI_FN(ptrv->get_value()), (void_fun ? nullptr : &result), values.data());
    
    if (void_fun)
        return nullptr;
    else {
        return result_to_moss(result, return_type, err);
    }
}