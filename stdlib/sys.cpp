#include "sys.hpp"
#include "moss.hpp"
#include "source.hpp"
#include <cstdlib>
#include <utility>

#if defined(__windows__)
    #include <windows.h>
#else
    extern "C" char **environ; // POSIX global
#endif

using namespace moss;
using namespace mslib;
using namespace sys;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& sys::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"add_path", [](Interpreter *, CallFrame* cf, Value *&) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return sys::add_path(cf->get_arg("path"), cf->get_arg("prepend"));
        }},
        {"getenv", [](Interpreter* vm, CallFrame* cf, Value *&err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 1 || args.size() == 2);
            return sys::getenv(vm, cf->get_arg("name"), cf->get_arg("def_val"), err);
        }},
        {"get_path", [](Interpreter*, CallFrame* cf, Value *&) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 0);
            auto pth = moss::get_lookup_path();
            std::vector<Value *> path;
            path.reserve(pth.size());
            for (auto p: pth) {
                path.push_back(StringValue::get(p));
            }
            return new ListValue(path);
        }},
        {"set_path", [](Interpreter *vm, CallFrame* cf, Value *&err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 1);
            return sys::set_path(vm, cf->get_args()[0].value, err);
        }},
        {"platform", [](Interpreter*, CallFrame* cf, Value *&err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 0);
            return sys::platform(cf, err);
        }},
    };
    return registry;
}

// Useful later for list of all
/*std::vector<std::pair<ustring, ustring>> get_all_env_vars() {
    std::vector<std::pair<ustring, ustring>> vars;
#if defined(__windows__)
    // Windows: use Unicode-safe version
    LPWCH envBlock = GetEnvironmentStringsW();
    if (!envBlock) return vars;

    LPWCH var = envBlock;
    while (*var) {
        std::wstring wvar(var);
        size_t eqPos = wvar.find(L'=');
        if (eqPos != std::wstring::npos) {
            ustring key(wvar.begin(), wvar.begin() + eqPos);
            ustring value(wvar.begin() + eqPos + 1, wvar.end());
            vars.emplace_back(key, value);
        }
        var += wcslen(var) + 1;
    }

    FreeEnvironmentStringsW(envBlock);
#else
    // POSIX: iterate over environ
    for (char **env = ::environ; *env != nullptr; ++env) {
        ustring entry(*env);
        size_t eqPos = entry.find('=');
        if (eqPos != ustring::npos) {
            vars.emplace_back(entry.substr(0, eqPos), entry.substr(eqPos + 1));
        }
    }
#endif
    return vars;
}*/

Value *sys::set_path(Interpreter *vm, Value *paths, Value *&err) {
    auto arg_path = mslib::get_list(paths);
    std::vector<ustring> new_paths;
    new_paths.reserve(arg_path.size());
    for (auto p: arg_path) {
        auto psv = dyn_cast<StringValue>(p);
        if (!psv) {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::SET_PATH_NOT_STR, p->get_type()->get_name().c_str()));
            return nullptr;
        }
        new_paths.push_back(psv->get_value());
    }
    std::vector<ustring> &curr_paths = moss::get_lookup_path();
    curr_paths = std::move(new_paths);
    return nullptr;
}

Value *sys::add_path(Value *path, Value *prepend) {
    auto arg_path = mslib::get_string(path);
    auto prep = mslib::get_bool(prepend);
    std::vector<ustring> &curr_paths = moss::get_lookup_path();
    if (prep) {
        curr_paths.insert(curr_paths.begin(), arg_path);
    } else {
        curr_paths.push_back(arg_path);
    }
    return nullptr;
}

Value *sys::platform(CallFrame *cf, Value *&err) {
    static bool initialized(false);
    static Value *result = nullptr;

    if (!initialized) {
        auto platform_enum = mslib::get_enum("Platform", cf, err);
        if (!platform_enum)
            return nullptr;
    #ifdef __windows__
        ustring curr_platform = "Windows";
    #elif defined(__APPLE__) // This will be true for any apple OS
        ustring curr_platform = "Darwin";
    #elif defined(__linux__)
        ustring curr_platform = "Linux";
    #else
        #warning "Compiling platform lib on unknown OS!"
        ustring curr_platform = "Unknown";
    #endif
        for (auto ev : platform_enum->get_values()) {
            if (ev->get_name() == curr_platform) {
                result = ev;
                initialized = true;
                break;
            }
        }
    }
    assert(result && "No platform was matched");
    return result;
}

Value *sys::getenv(Interpreter *vm, Value *name, Value *def_val, Value *&err) {
    assert(isa<StringValue>(name) && "Name is not string");
    const char* value = std::getenv(name->as_string().c_str());

    if (!value) {
        if (def_val)
            return def_val;
        err = mslib::create_key_error(diags::Diagnostic(*vm->get_src_file(), diags::KEY_NOT_FOUND, name->as_string().c_str()));
        return nullptr;
    }

    return StringValue::get(ustring(value));
}

void sys::init_constants(Interpreter *vm) {
    auto gf = vm->get_global_frame();
    Value *err = nullptr;

    // sys.version:String
    auto version_reg = mslib::get_global_register_of(vm, "version");
    ustring version = MOSS_VERSION;
#ifndef NDEBUG
    version += " (DEBUG build)";
#endif
    gf->store(version_reg, StringValue::get(version));
    // sys.version

    // sys.version_info
    auto version_info_space = mslib::get_space("version_info", vm, err);
    if (err)
        opcode::raise(err);
    version_info_space->set_attr("major", IntValue::get(MOSS_VERSION_MAJOR));
    version_info_space->set_attr("minor", IntValue::get(MOSS_VERSION_MINOR));
    version_info_space->set_attr("patch", IntValue::get(MOSS_VERSION_PATCH));
#ifndef NDEBUG
    version_info_space->set_attr("build_type", StringValue::get("debug"));
#else
    version_info_space->set_attr("build_type", StringValue::get("release"));
#endif
    // sys.version_info
}