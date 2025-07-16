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
            return sys::platform(vm, err);
        }},
    };
    return registry;
}

Value *sys::platform(Interpreter *vm, Value *&err) {
    return nullptr;
}