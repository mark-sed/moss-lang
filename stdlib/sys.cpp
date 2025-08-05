#include "sys.hpp"
#include "moss.hpp"

using namespace moss;
using namespace mslib;
using namespace sys;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& sys::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"platform", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 0);
            return sys::platform(vm, cf, err);
        }},
    };
    return registry;
}

void sys::init_constants(Interpreter *vm) {
    auto gf = vm->get_global_frame();
    Value *err = nullptr;

    // sys.version:String
    auto version_reg = mslib::get_constant_register(vm, "version");
    ustring version = MOSS_VERSION;
#ifndef NDEBUG
    version += " (DEBUG build)";
#endif
    gf->store(version_reg, new StringValue(version));
    // sys.version

    // sys.version_info
    auto version_info_space = mslib::get_space("version_info", vm, err);
    if (err)
        opcode::raise(err);
    version_info_space->set_attr("major", new IntValue(MOSS_VERSION_MAJOR));
    version_info_space->set_attr("minor", new IntValue(MOSS_VERSION_MINOR));
    version_info_space->set_attr("patch", new IntValue(MOSS_VERSION_PATCH));
#ifndef NDEBUG
    version_info_space->set_attr("build_type", new StringValue("debug"));
#elif
    version_info_space->set_attr("build_type", new StringValue("release"));
#endif
    // sys.version_info
}

Value *sys::platform(Interpreter *vm, CallFrame *cf, Value *&err) {
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