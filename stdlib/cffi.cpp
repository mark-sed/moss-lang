#include "cffi.hpp"
#include "values_cpp.hpp"
#include <unordered_map>
#include <unordered_set>
#include <ffi.h>
#ifndef __windows__
#include <dlfcn.h>
#else
#include <windows.h>
#endif

using namespace moss;
using namespace mslib;
using namespace cffi;
using namespace t_cpp;

union FFIResult {
    int cint;
    unsigned int cuint;
    int8_t cint8_t;
    uint8_t cuint8_t;
    int16_t cint16_t;
    uint16_t cuint16_t;
    int32_t cint32_t;
    uint32_t cuint32_t;
    int64_t cint64_t;
    uint64_t cuint64_t;
    long clong;
    long culong;
    bool cbool;
    short cshort;
    unsigned short cushort;
    char cchar;
    unsigned cuchar;
    float cfloat;
    double cdouble;
    void *cvoid_star;
};

const std::unordered_map<std::string, mslib::mslib_dispatcher>& cffi::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"()", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            return cffi::call(vm, cf, err);
        }},
        {"call", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            return cffi::call(vm, cf, err);
        }},
        {"call_moss", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto ret_v = cffi::call(vm, cf, err);
            if (err)
                return nullptr;
            if (auto c = dyn_cast<t_cpp::CppValue>(ret_v)) {
                // C++ classes are marked sealed so we can just check with isa
                return c->to_moss();
            }
            assert(false && "Returned call value is not a cpp value");
            return nullptr;
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

static Value *create_CFFIError(Interpreter *vm, CallFrame *cf, ustring msg, Value *&err) {
    return mslib::call_constructor(vm, cf, "CFFIError", {StringValue::get(msg)}, err);
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
static Value *windows_dlopen(Interpreter *vm, CallFrame *cf, ustring path, Value *&err) {
    HMODULE handle = LoadLibrary(path.c_str());
    if (!handle) {
        // TODO: Add error from loadlibrary
        err = create_CFFIError(vm, cf, "Loading library has failed.\n", err);
        return nullptr;
    }
    return new t_cpp::CVoidStarValue(handle);
}
#else
static Value *posix_dlopen(Interpreter *vm, CallFrame *cf, ustring path, Value *&err) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        const char *dlsym_error = dlerror();
        err = create_CFFIError(vm, cf, ustring(dlsym_error)+".\n", err);
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
    handle = windows_dlopen(vm, cf, path_str->get_value(), err);
#else
    handle = posix_dlopen(vm, cf, path_str->get_value(), err);
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
        {"cuint",    &ffi_type_uint},
        {"cint8_t",    &ffi_type_sint8},
        {"cuint8_t",    &ffi_type_uint8},
        {"cint16_t",    &ffi_type_sint16},
        {"cuint16_t",    &ffi_type_uint16},
        {"cint32_t",    &ffi_type_sint32},
        {"cuint32_t",    &ffi_type_uint32},
        {"cint64_t",    &ffi_type_sint64},
        {"cuint64_t",    &ffi_type_uint64},
        {"clong",   &ffi_type_slong},
        {"culong",   &ffi_type_ulong},
        {"cbool",    &ffi_type_uint8},
        {"cshort",  &ffi_type_sshort},
        {"cushort",  &ffi_type_ushort},
        {"cchar",   &ffi_type_schar},
        {"cuchar",   &ffi_type_uchar},
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
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_CPP_MOSS_VALUE, type->get_name().c_str()));
        } else {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_KNOWN_TYPE_CONV_TO_C, type->get_name().c_str()));
        }
        return nullptr;
    }
}

static CppValue *new_cpp_value(FFIResult result, Value *type, Value *&err) {
    assert(type != BuiltIns::Cpp::CVoid && "invoked with void");
    if (type == BuiltIns::Cpp::CInt)
        return new CIntValue(result.cint);
    if (type == BuiltIns::Cpp::CUInt)
        return new CUIntValue(result.cuint);
    if (type == BuiltIns::Cpp::CShort)
        return new CShortValue(result.cshort);
    if (type == BuiltIns::Cpp::CUShort)
        return new CUShortValue(result.cushort);
    if (type == BuiltIns::Cpp::CInt8_t)
        return new CInt8_tValue(result.cint8_t);
    if (type == BuiltIns::Cpp::CUInt8_t)
        return new CUInt8_tValue(result.cuint8_t);
    if (type == BuiltIns::Cpp::CInt16_t)
        return new CInt16_tValue(result.cint16_t);
    if (type == BuiltIns::Cpp::CUInt16_t)
        return new CUInt16_tValue(result.cuint16_t);
    if (type == BuiltIns::Cpp::CInt32_t)
        return new CInt32_tValue(result.cint32_t);
    if (type == BuiltIns::Cpp::CUInt32_t)
        return new CUInt32_tValue(result.cuint32_t);
    if (type == BuiltIns::Cpp::CInt64_t)
        return new CInt64_tValue(result.cint64_t);
    if (type == BuiltIns::Cpp::CUInt64_t)
        return new CUInt64_tValue(result.cuint64_t);
    if (type == BuiltIns::Cpp::CBool)
        return new CBoolValue(result.cbool);
    if (type == BuiltIns::Cpp::CLong)
        return new CLongValue(result.clong);
    if (type == BuiltIns::Cpp::CDouble)
        return new CDoubleValue(result.cdouble);
    if (type == BuiltIns::Cpp::CFloat)
        return new CFloatValue(result.cfloat);
    if (type == BuiltIns::Cpp::CCharStar)
        return new CCharStarValue((static_cast<char *>(result.cvoid_star)));
    if (type == BuiltIns::Cpp::CVoidStar)
        return new CVoidStarValue(result.cvoid_star);
    if (type == BuiltIns::Cpp::CChar)
        return new CCharValue(result.cchar);
    if (type == BuiltIns::Cpp::CUChar)
        return new CUCharValue(result.cuchar);

    // This erorr should not really happen and return type should be checked in cfun
    err = mslib::create_not_implemented_error("Conversion for returned type is not yet implemented in cffi\n");
    return nullptr;
}

static Value *cpp_type_to_moss_type(Value *type) {
    std::unordered_set<Value *> int_types{BuiltIns::Cpp::CInt, BuiltIns::Cpp::CLong,
                                          BuiltIns::Cpp::CInt8_t, BuiltIns::Cpp::CInt16_t,
                                          BuiltIns::Cpp::CInt32_t, BuiltIns::Cpp::CInt64_t,
                                          BuiltIns::Cpp::CShort,
                                          BuiltIns::Cpp::CUInt, BuiltIns::Cpp::CULong,
                                          BuiltIns::Cpp::CUInt8_t, BuiltIns::Cpp::CUInt16_t,
                                          BuiltIns::Cpp::CUInt32_t, BuiltIns::Cpp::CUInt64_t,
                                          BuiltIns::Cpp::CUShort};
    if (int_types.find(type) != int_types.end())
        return BuiltIns::Int;
    if (type == BuiltIns::Cpp::CFloat || type == BuiltIns::Cpp::CDouble)
        return BuiltIns::Float;
    if (type == BuiltIns::Cpp::CBool)
        return BuiltIns::Bool;
    if (type == BuiltIns::Cpp::CCharStar)
        return BuiltIns::String;

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
    if (!func) {
        err = mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_FIND_FFUN, name_s.c_str()));
        return nullptr;
    }
#endif

    ffi_cif cif;
    std::vector<ffi_type *> *args = new std::vector<ffi_type *>();
    std::vector<FunValueArg *> fargs;
    int counter = 1;
    // We need to store args to then be able to delete them.
    // TODO: Create special value just for this to then not delete void *, but the actual type.
    auto args_ptr = new t_cpp::CVoidStarValue(args, true);
    for (auto a: argst) {
        auto convv = get_ffi_type(a, vm, err);
        if (!convv)
            return nullptr;
        args->push_back(convv);
        std::vector<Value *> call_arg_types{a};
        if (auto cpp_type = cpp_type_to_moss_type(a))
            call_arg_types.push_back(cpp_type);
        fargs.push_back(new FunValueArg("arg"+std::to_string(counter), call_arg_types));
        ++counter;
    }
    // Add this since this is a method over an object
    fargs.push_back(new FunValueArg("this", {}));
    ffi_type *ffi_ret_type = get_ffi_type(return_type, vm, err);
    if (!ffi_ret_type)
        return nullptr;

    auto prep_stat = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args->size(), ffi_ret_type, args->data());
    if (prep_stat != ffi_status::FFI_OK) {
        err = create_not_implemented_error("cffi.define failed, but exception is not implemented.\n");
        return nullptr;
    }
    
    auto func_v = new t_cpp::CVoidStarValue(func);
    auto cif_v = new t_cpp::Ffi_cifValue(cif);

    auto cfun_handle = mslib::call_constructor(vm, cf, "ForeignFunction", {func_v, cif_v, name, return_type, arg_types, args_ptr}, err);
    if (!cfun_handle)
        return nullptr;
    
    auto old_call = cfun_handle->get_attr("call", vm);
    FunValue *old_call_fun = dyn_cast<FunValue>(old_call);
    assert(old_call_fun && "Call function is not a function? Perhaps a FunList");

    auto owner = dyn_cast<ModuleValue>(old_call_fun->get_owner());
    assert(owner && "owner of python module function not set?");
    // A new function as to be created otherwise we would override it for all the objects as they
    // share the one FunValue for call.
    auto new_call_fun = new FunValue("call", fargs, owner->get_vm(), old_call_fun->get_body_addr(), owner);
    for (auto [k, v]: old_call_fun->get_annotations())
        new_call_fun->annotate(k, v);
    cfun_handle->set_attr("call", new_call_fun);

    auto old_op_call = cfun_handle->get_attr("()", vm);
    FunValue *old_op_call_fun = dyn_cast<FunValue>(old_op_call);
    assert(old_op_call_fun && "() function is not a function? Perhaps a FunList");
    auto new_op_fun = new FunValue("()", fargs, owner->get_vm(), old_op_call_fun->get_body_addr(), owner);
    for (auto [k, v]: old_op_call_fun->get_annotations())
        new_op_fun->annotate(k, v);
    cfun_handle->set_attr("()", new_op_fun);

    auto old_call_moss = cfun_handle->get_attr("call_moss", vm);
    FunValue *old_call_moss_fun = dyn_cast<FunValue>(old_call_moss);
    assert(old_call_moss_fun && "call_moss function is not a function? Perhaps a FunList");
    auto new_call_moss_fun = new FunValue("call_moss", fargs, owner->get_vm(), old_call_moss_fun->get_body_addr(), owner);
    for (auto [k, v]: old_call_moss_fun->get_annotations())
        new_call_moss_fun->annotate(k, v);
    cfun_handle->set_attr("call_moss", new_call_moss_fun);

    ths->set_attr(name_s, cfun_handle);
    return nullptr;
}

Value *cffi::call(Interpreter *vm, CallFrame *cf, Value *&err) {
    auto args = cf->get_args();
    auto ths = cf->get_arg("this");
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

    std::vector<void *> values;
    for (auto a: args) {
        if (a.name == "this")
            continue;
        values.push_back(a.value->get_data_pointer());
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
        return new_cpp_value(result, return_type, err);
    }
}