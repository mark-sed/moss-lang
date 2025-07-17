#include "sys.hpp"

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